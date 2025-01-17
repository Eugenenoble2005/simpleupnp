#include "SSDPServer.h"
#include "HTTPServer.h"
#include <array>
#include <cstdio>
#include <locale>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include "../helpers/logger.h"

Server::SSDPServer::SSDPServer() {
    InitUdpSocket();
    // listen for search requests over UDP on seperate thread
    std::thread listener_thread(&Server::SSDPServer::ListenOnUdpSocket, this);

    // start http server on seperate thread
    std::thread http_server_thread(&Server::HTTPServer::StartListen, http_server.get());
    // start http server on seperate thread

    // run threads independently
    listener_thread.detach();
    http_server_thread.detach();
}

// copied with love  from stackoveflow
void Server::SSDPServer::InitUdpSocket() {
    // create UDP Socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

    // enable reuse addr
    int reuse = 1;
    setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    memset((char*)&groupSock, 0, sizeof(groupSock));

    groupSock.sin_family      = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr(SSDP_ADDR.c_str());
    groupSock.sin_port        = htons(SSDP_PORT);

    char loopch = 0;

    setsockopt(udpSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loopch, sizeof(loopch));

    memset((char*)&localSock, 0, sizeof(localSock));
    localSock.sin_family      = AF_INET;
    localSock.sin_port        = htons(SSDP_PORT);
    localSock.sin_addr.s_addr = INADDR_ANY;

    bind(udpSocket, (struct sockaddr*)&localSock, sizeof(localSock));

    group.imr_multiaddr.s_addr = inet_addr(SSDP_ADDR.c_str());
    group.imr_interface.s_addr = INADDR_ANY;

    setsockopt(udpSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group, sizeof(group));
}

/**
INFORM OTHER DEVICES WE ARE JOINING THE NETWORK
**/
void Server::SSDPServer::Advertise(std::string NTS, bool advertiseForSearch, struct sockaddr_in* sock_other) {
    const u_int device_count = 9;
    if (upnp_device == nullptr) {
        LogError("Could not init UPNP Device. Aborting...");
        exit(0);
    }
    //root device notifications
    struct Server::NTUSNValuePair root_device_one;
    root_device_one.NT  = "upnp:rootdevice";
    root_device_one.USN = "uuid:" + upnp_device->GUID + "::upnp:rootdevice";

    struct Server::NTUSNValuePair root_device_two;
    root_device_two.NT  = "uuid:" + upnp_device->GUID;
    root_device_two.USN = "uuid:" + upnp_device->GUID;

    struct Server::NTUSNValuePair root_device_three;
    root_device_three.NT  = "urn:schemas-upnp-org:device:MediaServer:1";
    root_device_three.USN = "uuid:" + upnp_device->GUID + "::urn:schemas-upnp-org:device:MediaServer:1";

    //embedded device notifications
    struct Server::NTUSNValuePair embedded_device_one;
    embedded_device_one.NT  = "uuid:" + upnp_device->GUID;
    embedded_device_one.USN = "uuid:" + upnp_device->GUID;

    struct Server::NTUSNValuePair embedded_device_two;
    embedded_device_two.NT  = "urn:schemas-upnp-org:device:MediaServer:1";
    embedded_device_two.USN = "uuid:" + upnp_device->GUID + "::urn:schemas-upnp-org:device:MediaServer:1";

    //service notifications
    struct Server::NTUSNValuePair service_one;
    service_one.NT  = "urn-schemas-upnp-org:service:ConnectionManager:1";
    service_one.USN = "uuid:" + upnp_device->GUID + "::urn:schemas-upnp-org:service:ConnectionManager:1";

    struct Server::NTUSNValuePair service_two;
    service_one.NT  = "urn-schemas-upnp-org:service:ContentDirectory:1";
    service_two.USN = "uuid:" + upnp_device->GUID + "::urn-schemas-upnp-org:service:ContentDirectory:1";
    
     struct Server::NTUSNValuePair service_three;
    service_three.NT  = "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1";
    service_three.USN = "uuid" + upnp_device->GUID + "::urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1";

    const struct Server::NTUSNValuePair devices[device_count] = {root_device_one,     root_device_two, root_device_three, embedded_device_one,
                                                                 embedded_device_two, service_one,     service_two,service_three};

    for (int i = 0; i < device_count; ++i) {
        const struct NTUSNValuePair device = devices[i];
        SendDatagram(NotifcationMessage(device.NT, device.USN, NTS, advertiseForSearch).c_str(), sock_other);
    }
}

void Server::SSDPServer::SendDatagram(const char* messageStream, struct sockaddr_in* sock_other) {
    //multicast notification if there is no reciever address provider
    int responseSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_other == nullptr) {
        sendto(udpSocket, messageStream, strlen(messageStream), 0, (struct sockaddr*)&groupSock, sizeof(groupSock));
    } else {
        //unicast otherwise
        sendto(responseSocket, messageStream, strlen(messageStream), 0, (struct sockaddr*)sock_other, sizeof(*sock_other));
    }
}
std::string Server::SSDPServer::NotifcationMessage(std::string NT, std::string USN, std::string NTS, bool isSearchResponse) {
    std::string notifcation_message_template = "NOTIFY * HTTP/1.1\r\n"
                                               "HOST: 239.255.255.250:1900\r\n"
                                               "CACHE-CONTROL: max-age = 1800\r\n"
                                               "LOCATION: http://" +
        upnp_device->IPV4_ADDRESS +
        ":2005/desc.xml\r\n"
        "NT: " +
        NT +
        "\r\n"
        "NTS: " +
        NTS +
        "\r\n"
        "SERVER: UPnP/1.0 DLNADOC/1.50 Platinum/1.0.5.13\r\n"
        "USN: " +
        USN +
        "\r\n"
        "BOOTID.UPNP.ORG: 0\r\n"
        "CONFIG.UPNP.ORG: 1\r\n"
        "\r\n";

    std::string search_response_message_template = "HTTP/1.1 200 OK\r\n"
                                                   "CACHE-CONTROL: max-age = 1800\r\n"
                                                   "EXT: \r\n"
                                                   "LOCATION: http://" +
        upnp_device->IPV4_ADDRESS +
        ":2005/desc.xml \r\n"
        "ST: " +
        NT +
        "\r\n"
        "SERVER: UPnP/1.0 DLNADOC/1.50 Platinum/1.0.5.13\r\n"
        "USN: " +
        USN +
        "\r\n"
        "BOOTID.UPNP.ORG: 0\r\n"
        "CONFIG.UPNP.ORG: 1\r\n"
        "\r\n";
    // return the appropriate template depending on whether or not we are
    // responing to a search
    return isSearchResponse ? search_response_message_template : notifcation_message_template;
}

/*
INFORM OTHER DEVICES WE ARE LEAVING THE NETWORK
*/
void Server::SSDPServer::GoodBye() {
    // Advertise with ssdp:byebye
    Advertise("ssdp:byebye", false);
    LogInfo("SSDP:BYEBYE BROADCASTED");
    http_server->CloseServer();
}

// listen for search requests
void Server::SSDPServer::ListenOnUdpSocket() {
    struct sockaddr_in si_other;
    socklen_t          slen = sizeof(si_other);
    char               buffer[1024];
    std::string        buffer_str;
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        recvfrom(udpSocket, buffer, 1024, 0, (struct sockaddr*)&si_other, &slen);
        buffer_str = buffer;
        if (buffer_str.find("M-SEARCH") != std::string::npos) {
            // found a search request, we must respond to it, NTS can be anything here
            // since it is not part of the search response header.
            //si_other pointer is passed becuase we must unicast the reponse of the search
            Advertise("any", true, &si_other);
            //specs say not to do this but it's the only way i could get some renderers to work like BubbleUPNP
            Hello();
            LogInfo("RESPONDING TO SSDP M-SEARCH");
        } else {
            // std::cout << buffer_str << std::endl;
        }
        buffer_str.clear();
    }
}

void Server::SSDPServer::Hello() {
    // Advertise with ssdp:hello
    Advertise("ssdp:alive", false);
    LogInfo("SSDP:ALIVE BROADCASTED");
}

Server::SSDPServer::~SSDPServer() {
    LogInfo("Destructing ssdp");
}

// UPNP DEVICE STRUCT
