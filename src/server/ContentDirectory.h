#ifndef CONTENT_DIRECTORY_H
#define CONTENT_DIRECTORY_H
#include <string>
namespace Server {
    enum ContentDirectoryAction {
        Browse,
        Invalid
    };

    class ContentDirectory {
      public:
        static void                   Control(std::string& request, std::stringstream& response);

        static ContentDirectoryAction GetAction(std::string& request);

      private:
        static void Browse(std::string& request, std::stringstream& response);
        static void ReadPhysicalDirectory(std::string root);
    };
}

#endif
