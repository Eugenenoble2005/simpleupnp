#ifndef SSDP_SERVER_H
#define SSDP_SERVER_H
#include "../helpers/uuid_generator.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
namespace Server {
struct UPNPDevice {
public:
  std::string USN;
  std::string GUID;
  std::string OS_VERSION;
  std::string LOCATION;
  UPNPDevice();
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
  const std::string SSDP_ADDR = "239.255.255.250";
  const int SSDP_PORT = 1900;
  struct in_addr localInterface;
  struct sockaddr_in groupSock;
  struct sockaddr_in localSock;
  struct ip_mreq group;
  const struct Server::UPNPDevice *upnp_device = new Server::UPNPDevice();

  void SendDatagram(const char *messageStream);
  int udpSocket;

  void InitUdpSocket();
  void Advertise(std::string NTS, bool advertiseForSearch);
  std::string NotifcationMessage(std::string NT, std::string USN,
                                 std::string NTS, bool isSearchResponse);
  void ListenOnUdpSocket();
};

} // namespace Server

#endif
