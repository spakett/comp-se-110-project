#include "INetworkRequestPort.h"
#include "INetworkCallbackPort.h"

#include <QNetworkAccessManager>
#include <QObject>

class QtNetworkClient : public QObject, public INetWorkRequestPort {

public:
    explicit QNetworkClient(
        INetworkCallbackPort& callback,
        QObject* parent = nullptr
    );

    void request(
        const std::string& requestId,
        const std::string& method,
        const std::string& apiIdentifier,
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& body = ""
    )

}