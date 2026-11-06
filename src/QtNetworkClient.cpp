#include "QtNetworkClient.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>

using namespace std;

QtNetworkClient::QtNetworkClient(QObject* parent) : QObject(parent) {
}

void QtNetworkClient::setCallback(INetworkCallbackPort& callback) {
    m_callback = &callback;
}

void QtNetworkClient::request(
    const string& requestId,
    const string& method,
    const string& apiIdentifier,
    const string& url,
    const map<std::string, std::string>& headers,
    const string& body
)
{
    const QUrl parsedUrl(QString::fromStdString(url));

    if (!parsedUrl.isValid() || parsedUrl.scheme() != QStringLiteral("https") || parsedUrl.host().isEmpty()) {
        m_callback->onNetworkError(requestId, apiIdentifier, "Invalid HTTPS URL");
        return;
    }

    QNetworkRequest networkRequest(parsedUrl);
    networkRequest.setTransferTimeout(30000);

    for (const auto& header : headers) {
        networkRequest.setRawHeader(
            QByteArray::fromStdString(header.first),
            QByteArray::fromStdString(header.second)
        );
    }

    QNetworkReply* reply = m_manager.sendCustomRequest(
        networkRequest,
        QByteArray::fromStdString(method),
        QByteArray::fromStdString(body)
    );

    INetworkCallbackPort* callback = m_callback;

    connect(reply, &QNetworkReply::finished, this,
        [callback, reply, requestId, apiIdentifier]() {
            const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (reply->error() != QNetworkReply::NoError) {
                const std::string errorMessage =
                    statusCode != 0
                    ? "HTTP status " + std::to_string(statusCode)
                    : reply->errorString().toStdString();

                callback->onNetworkError(requestId, apiIdentifier, errorMessage);
            } else {
                const std::string payload =
                    reply->readAll().toStdString();

                callback->onNetworkResponse(requestId, apiIdentifier, payload);
            }

            reply->deleteLater();
        });
}