#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "IDataParserPort.h"
#include "IDatabasePort.h"
#include "INetworkCallbackPort.h"
#include "INetworkRequestPort.h"
#include "IApiHandlerPort.h"

class ApiHandler final : public IApiHandlerPort, public INetworkCallbackPort {
public:
    ApiHandler(
        INetworkRequestPort& networkRequest,
        IDataParserPort& dataParser,
        IDatabasePort& database
    );
    
    void setCallback(IApiHandlerCallbackPort& callback);

    void searchLocation(const std::string& requestId, const std::string& addressQuery) override;
    
    void requestRoute(
        const std::string& requestId,
        const Coordinates& origin,
        const Coordinates& destination,
        TravelMode mode
    ) override;

    void requestAreaAirQuality(
        const std::string& requestId,
        const BoundingBox& area,
        const std::vector<Irritant>& irritants
    ) override;

    void requestHistoricalData(
        const std::string& requestId,
        const Coordinates& location,
        int daysPast,
        const std::vector<Irritant>& irritants
    ) override;

    void requestWeatherForecast(const std::string& requestId, const Coordinates& coords) override;

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
    enum class RequestType {
        LocationSearch,
        Route,
        AreaAirQuality,
        HistoricalAirQuality,
        WeatherForecast
    };

    struct RequestContext {
        RequestType type;
        std::string callbackRequestId;
        std::string apiIdentifier;
        std::string cacheKey;
    };

    void dispatchRequest(
        RequestType type,
        const std::string& callbackRequestId,
        const std::string& apiIdentifier,
        const std::string& method,
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& body = ""
    );
    std::string nextNetworkRequestId();
    bool deliverResponse(const RequestContext& context, const std::string& jsonPayload, bool reportError);

    IApiHandlerCallbackPort* m_callback = nullptr;
    INetworkRequestPort& m_networkRequest;
    IDataParserPort& m_dataParser;
    IDatabasePort& m_database;
    std::unordered_map<std::string, RequestContext> m_pendingRequests;
    std::uint64_t m_requestCounter = 0;
};