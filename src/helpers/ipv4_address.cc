#include "../helpers/ipv4_address.h"
#include "logger.h"
#include <ifaddrs.h> 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
std::string GetIpV4Address(){
  struct ifaddrs *interfaces = nullptr;
  struct ifaddrs *ifa = nullptr;

  void *tmpAddrPtr =  nullptr;
  //return loopback if it cant find anything more useful;
  std::string ipv4_address = "127.0.0.1";

  if(getifaddrs(&interfaces) == -1){
    LogError("Could not obtain Network interfaces. Proceeding anyway but might not work!");
    return "127.0.0.1";
  }

  for(ifa = interfaces; ifa != nullptr; ifa = ifa->ifa_next){
    if(ifa->ifa_addr == nullptr) continue;

    if(ifa->ifa_addr->sa_family == AF_INET){
      tmpAddrPtr = &((struct sockaddr_in * )ifa->ifa_addr)->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

      if(std::string(ifa->ifa_name) != "lo"){
        ipv4_address = addressBuffer;
        break;
      }
    }
  }
  freeifaddrs(interfaces);
  if(ipv4_address == "127.0.0.1")
    LogError("Could not obtain network interfaces. Are you connected to a wifi network?... Proceeding...");
  return ipv4_address;
}
