#include <atomic>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <ostream>
#include "server/SSDPServer.h"
#include "helpers/logger.h"

std::atomic<bool> keepRunning = true;
void signal_handler(int signal){
    if(signal == SIGINT || signal == SIGTERM){
        keepRunning = false;
    }
}
int main() {
    Server::SSDPServer ssdp_server;

    ssdp_server.Hello();
    std::signal(SIGINT,signal_handler);
    while (keepRunning) {
    }

    LogInfo("Termination Requested...");
    ssdp_server.GoodBye();
    LogInfo("Goodbye...");
}

