#include "../includes/HTTPServer.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <utility>

void Server::HTTPServer::StartServer() {
  m_socket = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(m_socket,SOL_SOCKET ,SO_REUSEADDR ,&opt ,sizeof(&opt) );
  if (m_socket < 0) {
    printf("Could not create socket for HTTP server");
    exit(0);
  }

  if (bind(m_socket, (sockaddr *)&m_socketAddress, m_socketAddress_len) < 0) {
    printf("Cannot connect socket to address");
    exit(0);
  }
}
Server::HTTPServer::HTTPServer() {
  m_socketAddress.sin_family = AF_INET;
  m_socketAddress.sin_port = htons(20054);
  // to allow LAN access
  m_socketAddress.sin_addr.s_addr = INADDR_ANY;
  m_socketAddress_len = sizeof(m_socketAddress);
  StartServer();
}
Server::HTTPServer::~HTTPServer() {}

void Server::HTTPServer::StartListen() {
  printf("Started listening");
  const int BUFFER_SIZE = 30270;
  if (listen(m_socket, 20) < 0) {
    printf("Failed to listen on socket");
    exit(0);
  }
  int bytes_recieved;
  while (true) {
    AcceptConnection(m_new_socket);
    char buffer[BUFFER_SIZE] = {0};
    bytes_recieved = read(m_new_socket, buffer, BUFFER_SIZE);
    if (bytes_recieved < 0) {
      printf("Failed to read bytes from connection");
    }
    HandleHttpRequest(buffer);
    close(m_new_socket);
  }
}
void Server::HTTPServer::AcceptConnection(int &new_socket) {
  new_socket =
      accept(m_socket, (sockaddr *)&m_socketAddress, &m_socketAddress_len);
  if (new_socket < 0) {
    printf("failed to accept incoming connections on http server");
  }
}

void Server::HTTPServer::HandleHttpRequest(char *buffer) {
  const char * log = "RECEIVED THE FOLLOWING BUFFER:";
  std::cout << log << std::endl;
  const std::string  reply_buffer = "<root>hello world from xml</root>";
  write(m_new_socket,reply_buffer.c_str(), size(reply_buffer));
}

void Server::HTTPServer::CloseServer()
{
  close(m_new_socket);
  close(m_socket);
}
