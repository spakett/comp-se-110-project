#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "IDatabasePort.h"
#include "INetworkCallbackPort.h"
#include "INetworkRequestPort.h"
#include "IApiHandlerPort.h"

class ApiHandler final : public IApiHandlerPort, public INetworkCallbackPort {
public:
    ApiHandler(
        INetworkRequestPort& networkRequest,
        IDatabasePort& database
    );
    
    void setCallback(IApiHandlerCallbackPort& callback);

    void requestAreaAirQuality(
        const std::string& requestId,
        const BoundingBox& area,
        const std::vector<Irritant>& irritants
    ) override;

    void onNetworkResponse(
        const std::string& requestId,
        const std::string& apiIdentifier,
        const std::string& jsonPayload
    ) override;

    void onNetworkError(
        const std::string& requestId,
        const std::string& apiIdentifier,
        const std::string& errorMessage
    ) override;

private:
    struct RequestContext {
        std::string cacheKey;
        std::vector<Coordinates> points;
        std::vector<Irritant> irritants;
    };

    void dispatchRequest(
        const std::string& requestId,
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::vector<Coordinates>& points,
        const std::vector<Irritant>& irritants
    );
    bool deliverResponse(const std::string& requestId, const RequestContext& context, const std::string& payload, bool reportError);

    IApiHandlerCallbackPort* m_callback = nullptr;
    INetworkRequestPort& m_networkRequest;
    IDatabasePort& m_database;
    std::unordered_map<std::string, RequestContext> m_pendingRequests;
};