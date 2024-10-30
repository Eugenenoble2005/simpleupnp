#include "ConnectionManager.h"
#include <iostream>
#include <sstream>

void Server::ConnectionManager::Control(std::string& request, std::stringstream& response, std::optional<ConnectionManagerAction> action, std::optional<int> response_socket) {
  std::cout << "Receieved a request to the connection manager" << std::endl;
}
