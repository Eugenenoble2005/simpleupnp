#ifndef CONTENT_DIRECTORY_H
#define CONTENT_DIRECTORY_H
#include <cstddef>
#include <string>
#include <vector>
namespace Server {
    enum ContentDirectoryAction {
        Browse,
        Invalid
    };
    enum MediaType{
      Video,
      Image,
      Music
    };
    struct PhysicalDirectoryItem{
      std::string itemName;
      std::string fullSystemPath;
      bool isContainer;

      //only important if the instance is not a container
      MediaType mediaType;
      std::string fileExtension;
    };
    class ContentDirectory {
      public:
        static  void                   Control(std::string& request, std::stringstream& response);

        static  ContentDirectoryAction GetAction(std::string& request);

      private:
       static  void Browse(std::string& request, std::stringstream& response);
         static std::vector<PhysicalDirectoryItem> ReadPhysicalDirectory(std::string root);
         static std::string BuildUBrowseXMLResponse(std::vector<PhysicalDirectoryItem> & pd_items);
    };
}

#endif
