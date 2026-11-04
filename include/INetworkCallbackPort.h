#include <string>

// Boost.Beast uses this to push async HTTP responses back to the Core
class INetworkCallbackPort {
public:
    virtual ~INetworkCallbackPort() = default;

    virtual void onNetworkResponse(const std::string& requestId, const std::string& apiIdentifier, const std::string& jsonPayload) = 0; 
    virtual void onNetworkError(const std::string& requestId, const std::string& apiIdentifier, const std::string& errorMessage) = 0;
};
