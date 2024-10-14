#include "../includes/SSDPServer.h"
#include "../includes/HTTPServer.h"
#include <array>
#include <cstdio>
#include <locale>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>

Server::SSDPServer::SSDPServer() {
  InitUdpSocket();
  // listen for search requests over UDP on seperate thread
  std::thread listener_thread(&Server::SSDPServer::ListenOnUdpSocket, this);

  // start http server on seperate thread
  std::thread http_server_thread(&Server::HTTPServer::StartListen,
                                 *http_server);
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
  setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse,
             sizeof(reuse));

  memset((char *)&groupSock, 0, sizeof(groupSock));

  groupSock.sin_family = AF_INET;
  groupSock.sin_addr.s_addr = inet_addr(SSDP_ADDR.c_str());
  groupSock.sin_port = htons(SSDP_PORT);

  char loopch = 0;

  setsockopt(udpSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch,
             sizeof(loopch));

  memset((char *)&localSock, 0, sizeof(localSock));
  localSock.sin_family = AF_INET;
  localSock.sin_port = htons(SSDP_PORT);
  localSock.sin_addr.s_addr = INADDR_ANY;

  bind(udpSocket, (struct sockaddr *)&localSock, sizeof(localSock));

  group.imr_multiaddr.s_addr = inet_addr(SSDP_ADDR.c_str());
  group.imr_interface.s_addr = INADDR_ANY;

  setsockopt(udpSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group,
             sizeof(group));
}

/**
INFORM OTHER DEVICES WE ARE JOINING THE NETWORK
**/
void Server::SSDPServer::Advertise(std::string NTS, bool advertiseForSearch) {
  const u_int device_count = 3;
  if (upnp_device == nullptr) {
    fprintf(stderr, "Could not init UPNP device. Exiting");
    exit(0);
  }
  // root device notifications
  // NT - USN KEY VALUE PAIR
  struct Server::NTUSNValuePair root_device_one;
  root_device_one.NT = "upnp:rootdevice";
  root_device_one.USN = "uuid:" + upnp_device->GUID + "::upnp:rootdevice";

  struct Server::NTUSNValuePair root_device_two;
  root_device_two.NT = "uuid:" + upnp_device->GUID;
  root_device_two.USN = "uuid:" + upnp_device->GUID;

  struct Server::NTUSNValuePair root_device_three;
  root_device_three.NT = "urn:schemas-upnp-org:device:MediaServer:1";
  root_device_three.USN = "uuid:" + upnp_device->GUID +
                          "::urn:schemas-upnp-org:device:MediaServer:1";

  const struct Server::NTUSNValuePair devices[device_count] = {
      root_device_one,
      root_device_two,
      root_device_three,
  };

  for (int i = 0; i < device_count; ++i) {
    const struct NTUSNValuePair device = devices[i];
    SendDatagram(
        NotifcationMessage(device.NT, device.USN, NTS, advertiseForSearch)
            .c_str());
  }
}

void Server::SSDPServer::SendDatagram(const char *messageStream) {
  sendto(udpSocket, messageStream, strlen(messageStream), 0,
         (struct sockaddr *)&groupSock, sizeof(groupSock));
}
std::string Server::SSDPServer::NotifcationMessage(std::string NT,
                                                   std::string USN,
                                                   std::string NTS,
                                                   bool isSearchResponse) {
  std::string notifcation_message_template =
      "NOTIFY * HTTP/1.1\r\n"
      "HOST: " +
      SSDP_ADDR + ":" + std::to_string(SSDP_PORT) +
      "\r\n"
      "CACHE-CONTROL: max-age=180\r\n"
      "LOCATION: http://192.168.100.2:20054/desc.xml\r\n"
      "NT: " +
      NT +
      "\r\n" // Type of device
      "NTS:" +
      NTS +
      "\r\n" // Notification type
      "USN: " +
      USN +
      "\r\n"
      "SERVER: UPnP/1.1 simpleupnp/1.0\r\n"
      "\r\n";

  std::string search_response_message_template =
      " HTTP/1.1\r\n"
      "CACHE-CONTROL: max-age=180\r\n"
      "LOCATION: http://192.168.100.2:20054/desc.xml\r\n"
      "ST:" + NT +
      "\r\n" // Notification type
      "USN: " +
      USN +
      "\r\n"
      "SERVER: UPnP/1.1 simpleupnp/1.0\r\n"
      "\r\n";
  // return the appropriate template depending on whether or not we are
  // responing to a search
  return isSearchResponse ? search_response_message_template
                          : notifcation_message_template;
}

/*
INFORM OTHER DEVICES WE ARE LEAVING THE NETWORK
*/
void Server::SSDPServer::GoodBye() {
  // Advertise with ssdp:byebye
  Advertise("ssdp:byebye", false);
}

// listen for search requests
void Server::SSDPServer::ListenOnUdpSocket() {
  struct sockaddr_in si_other;
  socklen_t slen = sizeof(si_other);
  char buffer[1024];
  std::string buffer_str;
  while (true) {
    recvfrom(udpSocket, buffer, 1024, 0, (struct sockaddr *)&si_other, &slen);
    buffer_str = buffer;
    //`std::cout << buffer_str << std::endl;
    if (buffer_str.find("M-SEARCH") != std::string::npos) {
      // found a search request, we must respond to it, NTS can be anything here
      // since it is not part of the search response header.
      Advertise("any", true);
    }
    buffer_str.clear();
  }
}

void Server::SSDPServer::Hello() {
  // Advertise with ssdp:hello
  Advertise("ssdp:hello", false);
}

Server::SSDPServer::~SSDPServer() {
  delete upnp_device;
}

// UPNP DEVICE STRUCT
