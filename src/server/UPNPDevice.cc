#include "SSDPServer.h"

Server::UPNPDevice::UPNPDevice() {
    //GUID = generate_uuid();
    //realised there's literally no reason to generate a random UUID each time.
    GUID = "2db225f8-14b3-4c97-b28e-b7e9bee15224";
}
