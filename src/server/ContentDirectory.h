#ifndef CONTENT_DIRECTORY_H
#define CONTENT_DIRECTORY_H
#include <string>
#include <vector>
namespace Server {
    enum ContentDirectoryAction {
        Browse,
        Invalid
    };
    struct PhysicalDirectoryItem{
      std::string itemName;
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
