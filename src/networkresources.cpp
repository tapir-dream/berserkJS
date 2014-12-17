#include "networkresources.h"
#include <QDebug>
#include <QHostInfo>

const qint64 MAX_REQUEST_POST_BODY_SIZE = 5 * 1024 * 1024; //最大5M

NetworkResources* NetworkResources::networkResources = 0;

NetworkResources* NetworkResources::getInstance()
{
    if (NetworkResources::networkResources == 0) {
        NetworkResources::networkResources = new NetworkResources();
    }
    return NetworkResources::networkResources;
}

void NetworkResources::clear()
{
    resourceList.clear();
}

QList<NetworkResources::resource> NetworkResources::getData()
{
    return resourceList;
}

void NetworkResources::addData(const QNetworkReply *reply,  QIODevice *device,
                               const QMap<QString, qint64> responseOtherData)
{
    resource res;
    QNetworkRequest request = reply->request();
    QNetworkAccessManager::Operation op = reply->operation();
    QString url = request.url().toString();

    QList<QByteArray> requestHeaderList = request.rawHeaderList();
    // 填充 request
    int headersSize = 0;
    for (int i = 0, c = requestHeaderList.length();  i < c; ++i) {
        QByteArray requestHeader = requestHeaderList.at(i);
        QByteArray requestHeaderValue = request.rawHeader(requestHeader);
        res.request.headers[QString(requestHeader)] = QString(requestHeaderValue);
        headersSize += requestHeader.size() + requestHeaderValue.size();
    }
    QHostInfo info = QHostInfo::fromName(reply->url().host());
    qDebug()<<info.addresses().isEmpty();
    if (!info.addresses().isEmpty()) {
        res.request.base["serverIPAddress"] = info.addresses().first().toString();
    }
    res.request.base["url"] = QVariant(url);
    res.request.base["FromCache"] = request.attribute(QNetworkRequest::SourceIsFromCacheAttribute);
    res.request.base["method"] =  (op == QNetworkAccessManager::GetOperation)
        ? QVariant("GET") : (reply->operation() == QNetworkAccessManager::PostOperation)
        ? QVariant("POST") : (reply->operation() == QNetworkAccessManager::HeadOperation)
        ? QVariant("HEAD") : (reply->operation() == QNetworkAccessManager::PutOperation)
        ? QVariant("PUT") : (reply->operation() == QNetworkAccessManager::DeleteOperation)
        ? QVariant("DELETE") : (reply->operation() == QNetworkAccessManager::CustomOperation)
        ? QVariant("CUSTOM"): QVariant("Unknown");

    QByteArray postData;
    if (op == QNetworkAccessManager::PostOperation) {
        postData = device->peek(MAX_REQUEST_POST_BODY_SIZE);
        res.request.base["bodySize"] = QVariant(postData.size());
        res.request.base["postData"] = postData.data();
    } else {
        res.request.base["bodySize"] = QVariant(0);
    }

    if (op == QNetworkAccessManager::GetOperation) {
        int index = url.indexOf("?");
        if (index != -1) {
            res.request.base["queryString"] = QVariant(url.right(url.length() - index - 1));
        } else {
            res.request.base["queryString"] = QVariant("");
        }
    } else {
        res.request.base["queryString"] = QVariant("");
    }

    res.request.base["headersSize"] = QVariant(headersSize);

    // ================= 完成 request 数据组装 ============


    // 填充 response
    QList<QByteArray> replyHeaderList = reply->rawHeaderList();
    headersSize = 0;
    for (int i = 0, c = replyHeaderList.length();  i < c; ++i) {
        QByteArray replyHeader = replyHeaderList.at(i);
        QByteArray replyHeaderValue = reply->rawHeader(replyHeader);
        res.response.headers[QString(replyHeader)] = QString(replyHeaderValue);
        headersSize += replyHeader.size() + replyHeaderValue.size();
    }
    res.response.base["status"] =  reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    res.response.base["statusText"] = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute);
    res.response.base["headersSize"] = QVariant(headersSize);
    qint64 bodySize = responseOtherData["size"] - res.response.base["headerSize"].toInt();
    res.response.base["bodySize"] = bodySize < 0 ? QVariant(0) :QVariant(bodySize);
    // ================= 完成 response 数据组装 ============


    // 填充 timings
    res.timings["blocked"] = responseOtherData["blocked"];
    res.timings["dns"] = responseOtherData["dns"];
    res.timings["connect"] = responseOtherData["connect"];
    res.timings["send"] = responseOtherData["send"];
    res.timings["wait"] = responseOtherData["wait"];
    res.timings["receive"] = responseOtherData["receive"];
    res.timings["startTime"] = responseOtherData["start"];
    res.timings["endTime"] = responseOtherData["end"];
    res.timings["ssl"] = url.indexOf("https://") == 0 ? 1 : -1;
    // ================= 完成 timings 数据组装 ============
    resourceList.append(res);
}


