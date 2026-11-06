#pragma once
#include "INetworkRequestPort.h"
#include "INetworkCallbackPort.h"

#include <QNetworkAccessManager>
#include <QObject>

class QtNetworkClient : public INetworkRequestPort, public QObject {

public:
    QtNetworkClient(QObject* parent = nullptr);
    void setCallback(INetworkCallbackPort& callback);

    void request(
        const std::string& requestId,
        const std::string& method,
        const std::string& apiIdentifier,
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& body = ""
    ) override;

private:
    INetworkCallbackPort* m_callback = nullptr;
    QNetworkAccessManager m_manager;
};