#pragma once
#include <string>
#include <map>

// API handler uses this to issue HTTP requests through the network adapter
class INetworkRequestPort {
public:
    virtual ~INetworkRequestPort() = default;
    virtual void request(
        const std::string& requestId,
        const std::string& method,
        const std::string& apiIdentifier,
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& body = ""
    ) = 0;
};
