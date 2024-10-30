#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <optional>
#include <string>
namespace Server{
  enum ConnectionManagerAction{
    
  };
  class ConnectionManager{
    public:
      static void Control(std::string & request, std::stringstream & response, std::optional<ConnectionManagerAction> action = std::nullopt, std::optional<int> response_socket = std::nullopt);

      static ConnectionManagerAction GetAction(std::string & request);
  };
}
#endif
