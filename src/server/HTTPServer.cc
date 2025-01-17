#include "HTTPServer.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <linux/close_range.h>
#include <netinet/in.h>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include "../helpers/logger.h"
#include "ConnectionManager.h"
#include "ContentDirectory.h"
#include "../helpers/encode_file_path.h"
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
Server::HTTPServer::~HTTPServer() {
    LogInfo("Destucing http server");
}

void Server::HTTPServer::StartListen() {
    const int BUFFER_SIZE = 30270;
    if (listen(m_socket, 20) < 0) {
        LogError("Failed to listen on UDP Server. Aborting...");
        exit(0);
    }
    while (true) {
        AcceptConnection(m_new_socket);
        std::string http_request;
        char        buffer[BUFFER_SIZE] = {0};
        int         bytes_recieved;
        //bytes_recieved           = read(m_new_socket, buffer, BUFFER_SIZE);
        while ((bytes_recieved = read(m_new_socket, buffer, BUFFER_SIZE)) > 0) {
            http_request.append(buffer, bytes_recieved);
            if (http_request.find("\r\n\r\n") != std::string::npos) {
                break;
            }
        }
        if (bytes_recieved < 0) {
            LogError("Failed to read bytes from connection");
        }
        /*Keep reading buffers until we match the content length*/
        size_t content_length_pos = http_request.find("Content-Length:");
        size_t body_start         = http_request.find("\r\n\r\n") + 4;
        int    content_length     = 0;

        if (content_length_pos != std::string::npos) {
            content_length = std::stoi(http_request.substr(content_length_pos + 15));
            http_request.reserve(body_start + content_length);
            while (http_request.size() < body_start + content_length) {
                bytes_recieved = read(m_new_socket, buffer, BUFFER_SIZE);
                if (bytes_recieved <= 0)
                    break;
                http_request.append(buffer, bytes_recieved);
            }
        }
        HandleHttpRequest(http_request.c_str());
        // close(m_new_socket);
    }
}
void Server::HTTPServer::AcceptConnection(int& new_socket) {
    new_socket = accept(m_socket, (sockaddr*)&m_socketAddress, &m_socketAddress_len);
    if (new_socket < 0) {
        LogError("Could not accept connections to HTTP server on port 20054. Aborting...");
        exit(0);
    }
}

void Server::HTTPServer::HandleHttpRequest(const char* buffer) {
    HttpRequest http_request              = ParseHttpRequest(buffer);
    bool        shouldWriteAndCloseSocket = true;
    // std::cout << "BUFFER WAS: " << buffer << std::endl;
    LogInfo("HTTP " + http_request.method + " RECIEVED TO: [" + http_request.uri + "]");
    std::stringstream response;
    if (http_request.uri == "/desc.xml" && http_request.method == "GET") {
        DeliverStaticFile("desc.xml", response);
    } else if (http_request.uri == "/ContentDirectory/scpd.xml" && http_request.method == "GET") {
        DeliverStaticFile("content-directory-scpd.xml", response);
    } else if (http_request.uri == "/ConnectionManager/scpd.xml" && http_request.method == "GET") {
        DeliverStaticFile("connection-manager-scpd.xml", response);
    } else if (http_request.uri == "/X_MS_MediaReceiverRegistrar/scpd.xml" && http_request.method == "GET") {
        DeliverStaticFile("ms-media-registrar-scpd.xml", response);
    } else if (http_request.uri == "/ContentDirectory/control.xml" && http_request.method == "POST") {
        Server::ContentDirectory::Control(http_request.content, response);
        //these are actually /SUBSCRIBE requests and not POST. Will deal with that later
    } else if (http_request.uri == "/ContentDirectory/event.xml" && http_request.method == "POST") {
        // std::cout << buffer << std::endl;
        ContentDirectory::Event(http_request.content, response);
    } else if (http_request.uri == "/ConnectionManager/event.xml" && http_request.method == "POST") {
        ContentDirectory::Event(http_request.content,response);
        // std::cout << buffer << std::endl;
    } else if (http_request.uri == "/ConnectionManager/control.xml" && http_request.method == "POST") {
        Server::ConnectionManager::Control(http_request.content, response);
    }
    ////Media resource request
    else if (http_request.uri.find("/importResource") != std::string::npos && http_request.method == "GET") {
        //get last url segment
        std::string       requestedResource;
        std::stringstream importResourceStream(http_request.uri);
        std::string       line;
        //get last url segment
        while (std::getline(importResourceStream, line, '/')) {
            requestedResource = line;
        }

        line.clear();
        //ignore thumbnail requests cause i aint finna deal with that now
        if (requestedResource.find("?") != std::string::npos) {
            std::cout << "A thumbain request?" << std::endl;
            return;
            requestedResource = requestedResource.substr(0, requestedResource.find("?"));
        }
        std::cout << requestedResource << std::endl;
        Server::ContentDirectory::Control(requestedResource, response, ContentDirectoryAction::ImportResource, m_new_socket);
        shouldWriteAndCloseSocket = false;
        return;
    }
    if (shouldWriteAndCloseSocket == true) {
        const std::string full_response = response.str();
        write(m_new_socket, full_response.c_str(), full_response.size());
        close(m_new_socket);
    }
    response.clear();
}

struct Server::HttpRequest Server::HTTPServer::ParseHttpRequest(const char* buffer) {
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
        if (line == "\r" || line == "\r\n" && checkSubsequentLinesForBody == false) {
            //next line will contain the body
            checkSubsequentLinesForBody = true;
        }
    }
    return http_request;
}

void Server::HTTPServer::DeliverStaticFile(std::string file_name, std::stringstream& response) {
    std::ifstream file(file_name);
    if (!file.is_open()) {
        LogError("Failed to read Content directory description XML File. Aborting...");
        return;
    }
    std::stringstream file_buffer;
    file_buffer << file.rdbuf();
    const std::string reply_buffer = file_buffer.str();
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: text/xml\r\n";
    response << "Server: " << "UPnP/1.0 DLANDOC/1.50 Platinum/1.0.5.13\r\n";
    response << "Content-Length: " << reply_buffer.size() << "\r\n";
    //  response << "Connection: close\r\n";
    response << "\r\n"; // End of headers
    response << reply_buffer;
    file.close();
}
void Server::HTTPServer::CloseServer() {
    LogInfo("Closing server Sockets");
    close(m_new_socket);
    close(m_socket);
}
