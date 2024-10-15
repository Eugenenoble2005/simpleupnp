#include <cstdio>
#include <iostream>
#include "server/SSDPServer.h"
int main() {
    Server::SSDPServer ssdp_server;

    ssdp_server.Hello();
    while (true) {}
}
