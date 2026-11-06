#include "api/airQualityService.h"

#include <QUrl>
#include <QUrlQuery>
#include <QStringList>
#include <map>
#include <stdexcept>

using namespace std;

// ACCEPTED VARIABLES: 
// timeVariable: "hourly", "current"
// pollutantVariables: "pm10", "pm2_5", "carbon_monoxide", "carbon_dioxide", "nitrogen_dioxide", "sulphur_dioxide", 
// "alder_pollen", "birch_pollen", "grass_pollen", "mugwort_pollen", "olive_pollen", "ragweed_pollen"

AirQualityService::AirQualityService(INetworkRequestPort& network): m_network(network) {
}
 
void AirQualityService::fetchAirQuality(const string& requestId, double latitude, double longitude, const vector<string>& pollutantVariables, const string& timeVariable) {
    // Define the endpoint
    QUrl url("https://air-quality-api.open-meteo.com/v1/air-quality");

    // Construct the API URL
    QUrlQuery query;

    query.addQueryItem("latitude", QString::number(latitude, 'f', 7));
    query.addQueryItem("longitude", QString::number(longitude, 'f', 7));

    // Add the selected variables
    QStringList selectedVariables;

    for (const auto& variable : pollutantVariables) {
        if (!selectedVariables.contains(QString::fromStdString(variable))) {
            selectedVariables.append(QString::fromStdString(variable));
        }
    }

    query.addQueryItem(
        QString::fromStdString(timeVariable),
        selectedVariables.join(',')
    );
    query.addQueryItem("timezone", "auto");

    url.setQuery(query);

    const map<string, string> headers = {{"Accept", "application/json"}};

    m_network.request(requestId, "GET", "air_quality", url.toString(QUrl::FullyEncoded).toStdString(), headers);
}
