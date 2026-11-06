#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "IDataParserPort.h"
#include "IDatabasePort.h"
#include "INetworkCallbackPort.h"
#include "INetworkRequestPort.h"
#include "IUiInputPort.h"
#include "IUiOutputPort.h"

class Core final : public IUiInputPort, public INetworkCallbackPort {
public:
    Core(
        IUiOutputPort& uiOutput,
        INetworkRequestPort& networkRequest,
        IDataParserPort& dataParser,
        IDatabasePort& database
    );

    void searchLocation(const std::string& addressQuery) override;
    void requestRoute(const Coordinates& origin, const Coordinates& destination, TravelMode mode) override;
    void setIrritantFilters(const std::vector<Irritant>& irritants) override;
    void requestAreaAirQuality(const BoundingBox& area) override;
    void requestHistoricalData(const Coordinates& location, int daysPast) override;
    void requestWeatherForecast(const Coordinates& coords) override;

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
        std::string cacheKey;
    };

    std::string nextRequestId();

    IUiOutputPort& m_uiOutput;
    INetworkRequestPort& m_networkRequest;
    IDataParserPort& m_dataParser;
    IDatabasePort& m_database;
    std::vector<Irritant> m_selectedIrritants;
    std::unordered_map<std::string, RequestContext> m_pendingRequests;
    std::uint64_t m_requestCounter = 0;
};