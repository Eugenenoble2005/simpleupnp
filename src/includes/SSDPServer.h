#ifndef SSDP_SERVER_H
#define SSDP_SERVER_H
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <asm-generic/socket.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace Server{
  class SSDPServer
  {
    public:
      SSDPServer();
      void Search();
      void Advertise();
      ~SSDPServer();
      
    private:
      const std::string SSDP_ADDR = "239.255.255.250";
      const int SSDP_PORT = 1900;
      struct in_addr localInterface;
      struct sockaddr_in groupSock;
      struct sockaddr_in localSock;
      struct ip_mreq group;    
      int udpSocket;

      void InitUdpSocket();
  };
}
  
#endif
