#include "SSDPServer.h"

Server::UPNPDevice::UPNPDevice() {
    GUID = generate_uuid();
}
