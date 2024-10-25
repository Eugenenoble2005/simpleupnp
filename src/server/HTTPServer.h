#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include <netinet/in.h>
#include <sstream>
#include <string>

namespace Server {

    struct HttpRequest {
        std::string method;
        std::string uri;
        std::string content;
    };
    class HTTPServer {
      public:
        HTTPServer();
        ~HTTPServer();
        void CloseServer();
        void StartListen();

      private:
        std::string        ip_address;
        int                m_port;
        int                m_socket;
        int                m_new_socket;
        long               m_incomingMessage;
        const int          opt = 1;
        struct sockaddr_in m_socketAddress;
        unsigned int       m_socketAddress_len;
        std::string        m_serverMessage;
        void               StartServer();
        void               AcceptConnection(int& new_socket);
        void               HandleHttpRequest(const char* buffer);
        struct HttpRequest ParseHttpRequest(const char* buffer);
        void               DeliverStaticFile(std::string file_name, std::stringstream &response);
        std::string        ContentDirectoryXMLResponse(std::string payload);
    };
} // namespace Server

#endif
