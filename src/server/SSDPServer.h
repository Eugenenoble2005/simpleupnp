#ifndef SSDP_SERVER_H
#define SSDP_SERVER_H
#include "../helpers/uuid_generator.h"
#include "HTTPServer.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
namespace Server {
    struct UPNPDevice {
      public:
        std::string USN;
        std::string GUID;
        std::string OS_VERSION;
        std::string LOCATION;
        std::string DESCRIPTION;
        std::string IPV4_ADDRESS;
        UPNPDevice();

      private:
        void SetDescription();
    };
    struct NTUSNValuePair {
        std::string USN;
        std::string NT;
    };

    class SSDPServer {
      public:
        SSDPServer();
        void Search();
        void GoodBye();
        void Hello();
        ~SSDPServer();

      private:
        const std::string                SSDP_ADDR = "239.255.255.250";
        const int                        SSDP_PORT = 1900;
        struct in_addr                   localInterface;
        struct sockaddr_in               groupSock;
        struct sockaddr_in               localSock;
        struct ip_mreq                   group;
        const struct std::unique_ptr<Server::UPNPDevice> upnp_device = std::make_unique<Server::UPNPDevice>();
        unsigned long  BOOT_ID = time(nullptr);
        std::unique_ptr<Server::HTTPServer>  http_server = std::make_unique<Server::HTTPServer>();

        void                             SendDatagram(const char* messageStream, struct sockaddr_in * sock_addr);
        int                              udpSocket;

        void                             InitUdpSocket();
        void                             Advertise(std::string NTS, bool advertiseForSearch, struct sockaddr_in * sock_addr = nullptr);
        std::string                      NotifcationMessage(std::string NT, std::string USN, std::string NTS, bool isSearchResponse);
        void                             ListenOnUdpSocket();
    };

} // namespace Server

#endif
