#include "ContentDirectory.h"
#include "../helpers/logger.h"
#include <tinyxml2.h>
//public facing control
void Server::ContentDirectory::Control(std::string& request, std::stringstream& response) {
    LogInfo("Received a content directory request");
    //process request

    ContentDirectoryAction content_directory_action = GetAction(request);
    switch (content_directory_action) {
        case Server::ContentDirectoryAction::Browse:
            Browse(request, response);
            break;
        default: return;
    }
    return;
}

void Server::ContentDirectory::Browse(std::string& request, std::stringstream& response) {
    tinyxml2::XMLDocument doc;
    //no need to verify nodes anymore cause it's been done.
    doc.Parse(request.c_str());

    tinyxml2::XMLElement* u_browse_element = doc.FirstChildElement("s:Envelope")->FirstChildElement("s:Body")->FirstChildElement("u:Browse");

    const std::string     ObjectID = u_browse_element->FirstChildElement("ObjectID")->GetText();

    const std::string     BrowseFlag = u_browse_element->FirstChildElement("BrowseFlag")->GetText();

    LogInfo(ObjectID + " " + BrowseFlag);
}

Server::ContentDirectoryAction Server::ContentDirectory::GetAction(std::string& request) {
    tinyxml2::XMLDocument doc;

    if (doc.Parse(request.c_str()) != tinyxml2::XML_SUCCESS) {
        LogWarning("Invalid XML Payload. Discarding request..");
        return ContentDirectoryAction::Invalid;
    }
    // First, navigate through the envelope structure
    tinyxml2::XMLElement* envelope = doc.FirstChildElement("s:Envelope");
    if (envelope == nullptr) {
        LogWarning("Invalid XML Payload. Discarding request..");
        return ContentDirectoryAction::Invalid;
    }

    tinyxml2::XMLElement* body = envelope->FirstChildElement("s:Body");
    if (body == nullptr) {
        LogWarning("Invalid XML Payload. Discarding request..");
        return ContentDirectoryAction::Invalid;
    }

    // Now look for the "u:Browse" element inside the body
    tinyxml2::XMLElement* u_browse_element = body->FirstChildElement("u:Browse");
    if (u_browse_element != nullptr) {
        return ContentDirectoryAction::Browse;
    }
    LogWarning("Invalid XML Payload. Discarding request..");
    //If it gets to this, someone fucked up.
    return ContentDirectoryAction::Invalid;
}
