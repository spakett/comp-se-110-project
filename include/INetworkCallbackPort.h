#include <string>

// Network adapter uses this to deliver async HTTP responses to the API handler
class INetworkCallbackPort {
public:
    virtual ~INetworkCallbackPort() = default;
    virtual void onNetworkResponse(const std::string& requestId, const std::string& apiIdentifier, const std::string& jsonPayload) = 0; 
    virtual void onNetworkError(const std::string& requestId, const std::string& apiIdentifier, const std::string& errorMessage) = 0;
};
