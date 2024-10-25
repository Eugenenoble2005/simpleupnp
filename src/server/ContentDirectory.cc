#include "ContentDirectory.h"
#include "../helpers/logger.h"
#include <algorithm>
#include <cstring>
#include <iterator>
#include <tinyxml2.h>
#include <sstream>
#include <filesystem>
#include <vector>
#include "../helpers/global.h"
#include "../helpers/uuid_generator.h"
#include "../helpers/escape_xml.h"
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
    tinyxml2::XMLElement*              u_browse_element = doc.FirstChildElement("s:Envelope")->FirstChildElement("s:Body")->FirstChildElement("u:Browse");
    const std::string                  ObjectID         = u_browse_element->FirstChildElement("ObjectID")->GetText();
    std::vector<PhysicalDirectoryItem> pd_items;
    //set content_directory to the user's chosen root unless another directory is requested by the renderer
    std::string content_directory = ObjectID == "0" ? Global::GetContentDirectory() : ObjectID;
    pd_items                      = ReadPhysicalDirectory(content_directory);
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
    //xml is escaped automatically
    Result->SetText(BuildUBrowseXMLResponse(pd_items).c_str());
    u_BrowseResponse->InsertEndChild(Result);

    //<NumberReturned>
    tinyxml2::XMLElement* NumberReturned = responseDocument.NewElement("NumberReturned");
    NumberReturned->SetText(pd_items.size());
    u_BrowseResponse->InsertEndChild(NumberReturned);

    //<TotalMatches>
    tinyxml2::XMLElement* TotalMatches = responseDocument.NewElement("TotalMatches");
    TotalMatches->SetText(pd_items.size());
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
}

Server::ContentDirectoryAction Server::ContentDirectory::GetAction(std::string& request) {
    tinyxml2::XMLDocument doc;
    if (doc.Parse(request.c_str()) != tinyxml2::XML_SUCCESS) {
        LogWarning("Invalid XML Payload. Discarding request..");
        return ContentDirectoryAction::Invalid;
    }
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

    tinyxml2::XMLElement* u_browse_element = body->FirstChildElement("u:Browse");
    if (u_browse_element != nullptr) {
        return ContentDirectoryAction::Browse;
    }
    LogWarning("Invalid XML Payload. Discarding request..");
    //If it gets to this, someone fucked up.
    return ContentDirectoryAction::Invalid;
}
std::vector<Server::PhysicalDirectoryItem> Server::ContentDirectory::ReadPhysicalDirectory(std::string root) {
    std::vector<PhysicalDirectoryItem> Items;
    const std::string                  supportedFileExtensions[] = {".mp4", ".mkv", ".avi", ".mp3", ".jpg", ".png", ".gif", ".webp"};
    if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
        LogWarning("Directory does not exist or is not readable. Ignoring Request");
        return Items;
    }
    //get all folders and files in this directory
    for (auto& item : std::filesystem::directory_iterator(root)) {
        struct PhysicalDirectoryItem pd_item;
        if (item.is_directory()) {
            pd_item.itemName       = item.path().filename();
            pd_item.fullSystemPath = item.path();
            pd_item.isContainer    = true;
            Items.push_back(pd_item);
        } else {
            //if not supported file type, continue
            if (std::find(std::begin(supportedFileExtensions), std::end(supportedFileExtensions), item.path().extension()) == std::end(supportedFileExtensions)) {
                continue;
            }
            pd_item.itemName       = item.path().filename();
            pd_item.fullSystemPath = item.path();
            pd_item.isContainer    = false;
            Items.push_back(pd_item);
        }
    }
    return Items;
}

std::string Server::ContentDirectory::BuildUBrowseXMLResponse(std::vector<PhysicalDirectoryItem>& pd_items) {
    tinyxml2::XMLDocument responseDocument;
    //<DIDL-Lite>
    tinyxml2::XMLElement* DIDL_Lite = responseDocument.NewElement("DIDL-Lite");
    responseDocument.InsertFirstChild(DIDL_Lite);
    DIDL_Lite->SetAttribute("xmlns", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
    DIDL_Lite->SetAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1");
    DIDL_Lite->SetAttribute("xmlns:upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");
    DIDL_Lite->SetAttribute("xmlns:dlna", "urn:schemas-dlna-org:metadata-1-0/");

    //currently working with only containers until i figure out exactly what i'm doing.
    for (auto& pd_item : pd_items) {
        //<container>
        tinyxml2::XMLElement* itemOrContainer;
        if (pd_item.isContainer) {
            itemOrContainer = responseDocument.NewElement("container");
            itemOrContainer->SetAttribute("searchable", "0");
        }
        else{
            itemOrContainer = responseDocument.NewElement("item");
        }
        itemOrContainer->SetAttribute("id", pd_item.fullSystemPath.c_str());
        itemOrContainer->SetAttribute("restricted", "1");
        itemOrContainer->SetAttribute("parentID", "0");

        //<dc:title>
        tinyxml2::XMLElement* dc_title = responseDocument.NewElement("dc:title");
        dc_title->SetText(pd_item.itemName.c_str());
        itemOrContainer->InsertEndChild(dc_title);

        //<dc:creator>
        tinyxml2::XMLElement* dc_creator = responseDocument.NewElement("dc:creator");
        dc_creator->SetText("Unknown");
        itemOrContainer->InsertEndChild(dc_creator);

        //<upnp:class>
        tinyxml2::XMLElement* upnp_class = responseDocument.NewElement("upnp:class");
        upnp_class->SetText(pd_item.isContainer ? "object.container" : "object.item.videoItem");
        itemOrContainer->InsertEndChild(upnp_class);

        //push to <DIDL-Lite> for every item
        DIDL_Lite->InsertEndChild(itemOrContainer);
    }
    tinyxml2::XMLPrinter printer;
    responseDocument.Print(&printer);
    std::string responseString = printer.CStr();

    return responseString;
}
