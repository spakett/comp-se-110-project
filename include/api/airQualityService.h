#include "INetworkRequestPort.h"

#include <vector>
#include <string>

class AirQualityService {
public:
    explicit AirQualityService(INetworkRequestPort& network);

    void fetchAirQuality(
        const std::string& requestId,
        double latitude,
        double longitude,
        const std::vector<std::string>& pollutantVariables,
        const std::string& timeVariable

    );

private:
    INetworkRequestPort& m_network;
};