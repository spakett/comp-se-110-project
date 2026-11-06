#include <string>
#include <map>

// Core uses this to command Qt Network to fetch data from APIs
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
