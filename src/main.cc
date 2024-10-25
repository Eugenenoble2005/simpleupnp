#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <ratio>
#include <thread>
#include "server/SSDPServer.h"
#include "helpers/logger.h"
#include "helpers/global.h"
std::atomic<bool> keepRunning = true;
void signal_handler(int signal){
    if(signal == SIGINT || signal == SIGTERM){
        keepRunning = false;
    }
}
int main() {
    Server::SSDPServer ssdp_server;
    Global::SetContentDirectoryRoot("/mnt");
    ssdp_server.Hello();
    std::signal(SIGINT,signal_handler);
    while (keepRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    LogInfo("Termination Requested...");
    ssdp_server.GoodBye();
    LogInfo("Goodbye...");
}

