#include "ContentDirectory.h"
#include "../helpers/logger.h"
#include <cstring>
#include <tinyxml2.h>
#include <sstream>
#include <filesystem>
#include "../helpers/global.h"
//public facing control
void Server::ContentDirectory::Control(std::string& request, std::stringstream& response) {
    LogInfo("Received a content directory request");
    //process request

    ContentDirectoryAction content_directory_action = GetAction(request);
    switch (content_directory_action) {
        case Server::ContentDirectoryAction::Browse: Browse(request, response); break;
        default: return;
    }
    return;
}

void Server::ContentDirectory::Browse(std::string& request, std::stringstream& response) {
    tinyxml2::XMLDocument doc;
    //no need to verify nodes anymore cause it's been done.
    doc.Parse(request.c_str());
    tinyxml2::XMLElement* u_browse_element = doc.FirstChildElement("s:Envelope")->FirstChildElement("s:Body")->FirstChildElement("u:Browse");
    const std::string     ObjectID         = u_browse_element->FirstChildElement("ObjectID")->GetText();
    if (ObjectID == "0") {
        //read root directory
        std::string root_directory = Global::GetContentDirectory();
    }
    //Build xml response;
    //standard SOAP
    tinyxml2::XMLDocument responseDocument;

    //<s:Envelope>
    tinyxml2::XMLElement* s_Envelope = responseDocument.NewElement("s:Envelope");
    s_Envelope->SetAttribute("s:encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
    s_Envelope->SetAttribute("xmlns:s", "http://schemas.xmlsoap.org/soap/envelope/");
    responseDocument.InsertFirstChild(s_Envelope);

    //<s:Body>
    tinyxml2::XMLElement* s_Body = responseDocument.NewElement("s:Body");
    s_Envelope->InsertEndChild(s_Body);

    //<u:BrowseResponse>
    tinyxml2::XMLElement* u_BrowseResponse = responseDocument.NewElement("u:BrowseResponse");
    u_BrowseResponse->SetAttribute("xmlns:u", "urn:schemas-upnp-org:service:ContentDirectory:1");
    s_Body->InsertEndChild(u_BrowseResponse);

    //<Result>
    tinyxml2::XMLElement* Result = responseDocument.NewElement("Result");
    u_BrowseResponse->InsertEndChild(Result);

    //<NumberReturned>
    tinyxml2::XMLElement* NumberReturned = responseDocument.NewElement("NumberReturned");
    NumberReturned->SetText("0");
    u_BrowseResponse->InsertEndChild(NumberReturned);

    //<TotalMatches>
    tinyxml2::XMLElement* TotalMatches = responseDocument.NewElement("TotalMatches");
    TotalMatches->SetText("0");
    u_BrowseResponse->InsertEndChild(TotalMatches);

    //<UpdateID>
    tinyxml2::XMLElement* UpdateID = responseDocument.NewElement("UpdateID");
    u_BrowseResponse->InsertEndChild(UpdateID);
    UpdateID->SetText("0");

    //print response
    tinyxml2::XMLPrinter printer;
    responseDocument.Print(&printer);
    const char* responseString = printer.CStr();

    //set http headers
    response << "HTTP/1.1 200 OK\r\n";
    response << "EXT: \r\n";
    response << "Content-Length: " << strlen(responseString) << "\r\n";
    response << "Content-Type: text/xml \r\n";
    response << "Server: UPnp/1.0 DLNADOC/1.50 Platinum/1.0.5.13 \r\n";
    response << "\r\n";

    response << responseString;
    LogInfo(response.str());
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
void Server::ContentDirectory::ReadPhysicalDirectory(std::string root) {
    if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
        LogWarning("Directory does not exist or is not readable. Ignoring Request");
        return;
    }
}
