#include "HTTPServer.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include "../helpers/logger.h"
void Server::HTTPServer::StartServer() {
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(&opt));
    if (m_socket < 0) {
        printf("Could not create socket for HTTP server");
        LogError("Could not bind HTTP Server to port 20054. Aborting...");
        exit(0);
    }

    if (bind(m_socket, (sockaddr*)&m_socketAddress, m_socketAddress_len) < 0) {
        LogError("Could not connect to HTTP socket. Aborting...");
        exit(0);
    }
}
Server::HTTPServer::HTTPServer() {
    m_socketAddress.sin_family = AF_INET;
    m_socketAddress.sin_port   = htons(20054);
    // to allow LAN access
    m_socketAddress.sin_addr.s_addr = INADDR_ANY;
    m_socketAddress_len             = sizeof(m_socketAddress);
    StartServer();
}
Server::HTTPServer::~HTTPServer() {}

void Server::HTTPServer::StartListen() {
    const int BUFFER_SIZE = 30270;
    if (listen(m_socket, 20) < 0) {
        printf("Failed to listen on socket");
        exit(0);
    }
    int bytes_recieved;
    while (true) {
        AcceptConnection(m_new_socket);
        char buffer[BUFFER_SIZE] = {0};
        bytes_recieved           = read(m_new_socket, buffer, BUFFER_SIZE);
        if (bytes_recieved < 0) {
            printf("Failed to read bytes from connection");
        }
        HandleHttpRequest(buffer);
        close(m_new_socket);
    }
}
void Server::HTTPServer::AcceptConnection(int& new_socket) {
    new_socket = accept(m_socket, (sockaddr*)&m_socketAddress, &m_socketAddress_len);
    if (new_socket < 0) {
        LogError("Could not accept connections to HTTP server on port 20054. Aborting...");
        exit(0);
    }
}

void Server::HTTPServer::HandleHttpRequest(char* buffer) {
    //std::cout << buffer << std::endl;
    LogInfo("HTTP GET RECIEVED");
    std::ifstream file("desc.xml");
    if (!file.is_open()) {
        LogError("Could not read description XML File. Aborting...");
        exit(0);
    }
    std::stringstream file_buffer;
    file_buffer << file.rdbuf();
    const std::string reply_buffer = file_buffer.str();

    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: text/xml\r\n";
    response << "Content-Length: " << reply_buffer.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n"; // End of headers

    // Add the actual XML content after headers
    response << reply_buffer;

    // Send the response to the client
    const std::string full_response = response.str();
    write(m_new_socket, full_response.c_str(), full_response.size());
}

void Server::HTTPServer::CloseServer() {
    close(m_new_socket);
    close(m_socket);
}
