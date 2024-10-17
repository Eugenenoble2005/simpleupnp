#include "HTTPServer.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
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
    m_socketAddress.sin_port   = htons(2005);
    // to allow LAN access
    m_socketAddress.sin_addr.s_addr = INADDR_ANY;
    m_socketAddress_len             = sizeof(m_socketAddress);
    StartServer();
}
Server::HTTPServer::~HTTPServer() {}

void Server::HTTPServer::StartListen() {
    const int BUFFER_SIZE = 30270;
    if (listen(m_socket, 20) < 0) {
        LogError("Failed to listen on UDP Server. Aborting...");
        exit(0);
    }
    int bytes_recieved;
    while (true) {
        AcceptConnection(m_new_socket);
        char buffer[BUFFER_SIZE] = {0};
        bytes_recieved           = read(m_new_socket, buffer, BUFFER_SIZE);
        if (bytes_recieved < 0) {
            LogError("Failed to read bytes from connection");
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
    HttpRequest http_request = ParseHttpRequest(buffer);
    LogInfo("HTTP " + http_request.method + " RECIEVED TO: [" + http_request.uri + "]");
    LogInfo(http_request.content);
    std::stringstream response;
    if (http_request.uri == "/desc.xml" && http_request.method == "GET") {
        std::ifstream file("desc.xml");
        if (!file.is_open()) {
            LogError("Could not read description XML File. Aborting...");
            exit(0);
        }
        std::stringstream file_buffer;
        file_buffer << file.rdbuf();
        const std::string reply_buffer = file_buffer.str();
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/xml\r\n";
        response << "Content-Length: " << reply_buffer.size() << "\r\n";
        response << "Connection: close\r\n";
        response << "\r\n"; // End of headers
        response << reply_buffer;
    } else if (http_request.uri == "/ContentDirectory/scpd.xml" && http_request.method == "GET") {
        std::ifstream file("content-directory-scpd.xml");
        if (!file.is_open()) {
            LogError("Failed to read Content directory description XML File. Aborting...");
            exit(0);
        }
        std::stringstream file_buffer;
        file_buffer << file.rdbuf();
        const std::string reply_buffer = file_buffer.str();
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/xml\r\n";
        response << "Content-Length: " << reply_buffer.size() << "\r\n";
        response << "Connection: close\r\n";
        response << "\r\n"; // End of headers
        response << reply_buffer;
    } else if (http_request.uri == "/ConnectionManager/scpd.xml" && http_request.method == "GET") {
        std::ifstream file("connection-manager-scpd.xml");
        if (!file.is_open()) {
            LogError("Failed to read Content directory description XML File. Aborting...");
            exit(0);
        }
        std::stringstream file_buffer;
        file_buffer << file.rdbuf();
        const std::string reply_buffer = file_buffer.str();
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/xml\r\n";
        response << "Content-Length: " << reply_buffer.size() << "\r\n";
        response << "Connection: close\r\n";
        response << "\r\n"; // End of headers
        response << reply_buffer;
        std::cout << response.str();
    }

    const std::string full_response = response.str();
    write(m_new_socket, full_response.c_str(), full_response.size());
    response.clear();
}

struct Server::HttpRequest Server::HTTPServer::ParseHttpRequest(char* buffer) {
    std::string        buffer_str(buffer);
    std::stringstream  buffer_stream(buffer_str);
    std::string        line;
    struct HttpRequest http_request;
    int                index                       = -1;
    bool               checkSubsequentLinesForBody = false;
    while (std::getline(buffer_stream, line)) {
        index++;
        //HTTP METHOD ACCORDING TO STANDARD
        if (index == 0) {
            //probably clumsy but the specs say i can only expect a GET OR A POST
            http_request.method = line.rfind("GET") == 0 ? "GET" : "POST";
            std::stringstream request_line(line);
            int               word_count = -1;
            std::string       item;
            while (std::getline(request_line, item, ' ')) {
                word_count++;
                //second word;
                if (word_count == 1) {
                    http_request.uri = item;
                }
            }
        }
        if (checkSubsequentLinesForBody == true) {

            http_request.content += line + "\n";
        }
        //check if headers have ended by finding the first empty line. Only do this for the first line incase there are other blank lines elsewhere.
        if (line == "\r" && checkSubsequentLinesForBody == false) {
            //next line will contain the body
            checkSubsequentLinesForBody = true;
        }
    }
    return http_request;
}

std::string Server::HTTPServer::ContentDirectoryXMLResponse(std::string payload) {}

void        Server::HTTPServer::CloseServer() {
    close(m_new_socket);
    close(m_socket);
}
