#include "../includes/HTTPServer.h"
#include <arpa/inet.h>
#include <cstdio>
#include <stdlib.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

void Server::HTTPServer::StartServer() {
  m_socket = socket(AF_INET, SOCK_STREAM, 0);
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
  //to allow LAN access
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
  while(true){
    AcceptConnection(m_new_socket);
    char buffer[BUFFER_SIZE] = {0};
    bytes_recieved = read(m_new_socket,buffer,BUFFER_SIZE);
    if(bytes_recieved < 0){
      printf("Failed to read bytes from connection");
    }
    printf("Recieved some data");
    printf("%s", buffer);
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
int main() {
  Server::HTTPServer http_server;
  http_server.StartListen();
}
