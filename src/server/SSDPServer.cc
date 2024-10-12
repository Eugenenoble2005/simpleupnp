#include "../includes/SSDPServer.h"
#include <cstdio>
#include <string>
#include <sys/socket.h>


Server::SSDPServer::SSDPServer()
{
  InitUdpSocket();
}

//copied from stackoveflow
void Server::SSDPServer::InitUdpSocket()
{
    //create UDP Socket
     udpSocket = socket(AF_INET,SOCK_DGRAM,0);

    //enable reuse addr
    int reuse = 1;
    setsockopt(udpSocket,SOL_SOCKET ,SO_REUSEADDR ,(char *)&reuse,sizeof(reuse));

    memset((char *)&groupSock , 0, sizeof(groupSock));

    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr(SSDP_ADDR.c_str());
    groupSock.sin_port = htons(SSDP_PORT);

    char loopch = 0;

    setsockopt(udpSocket,IPPROTO_IP ,IP_MULTICAST_LOOP , (char *)&loopch  ,sizeof(loopch) );

    memset((char *)&localSock , 0 , sizeof(localSock));
    localSock.sin_family = AF_INET;
    localSock.sin_port = htons(SSDP_PORT);
    localSock.sin_addr.s_addr = INADDR_ANY;

    bind(udpSocket,(struct sockaddr*)&localSock , sizeof(localSock) );

    group.imr_multiaddr.s_addr = inet_addr(SSDP_ADDR.c_str());
    group.imr_interface.s_addr = inet_addr("192.168.175.220");

    setsockopt(udpSocket,IPPROTO_IP ,IP_ADD_MEMBERSHIP ,(char *)&group , sizeof(group) );
}


void Server::SSDPServer::Advertise()
{
  std::string message = 
    "NOTIFY * HTTP/1.1\r\n"
    "HOST: " + SSDP_ADDR + ":" + std::to_string(SSDP_PORT) + "\r\n"
    "CACHE-CONTROL: max-age=180\r\n"
    "LOCATION: http://localhost:1000\r\n"
    "NT: upnp:rootdevice\r\n"    // Type of device
    "NTS: ssdp:alive\r\n"        // Notification type
    "USN: uuid:device-UUID::upnp:rootdevice\r\n"
    "SERVER: Arch/Linux UPnP/1.1 product/version\r\n"
    "\r\n";

  const char * messageStream = message.c_str();
  printf("%s", messageStream);
  //send message
  sendto(udpSocket,messageStream ,strlen(messageStream) ,0 ,(struct sockaddr *)&groupSock  , sizeof(groupSock) );

}

Server::SSDPServer::~SSDPServer()
{
  
}
