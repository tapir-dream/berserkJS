#include "customdownload.h"
#include "monitordata.h"
#include "monitordatamap.h"
#include "networkresources.h"
#include <QHostInfo>


CustomDownload::CustomDownload(QNetworkReply *reply,
                               QIODevice *device,
                               qint64 dnsLookupTime,
                               qint64 requestStartTime)
{

    this->isWaiting = true;
    this->requestStartTime = requestStartTime;
    this->waitingTime =  QDateTime::currentDateTime().toMSecsSinceEpoch();
    this->dnsLookupTime = dnsLookupTime;
    this->request = reply->request();
    this->reply = reply;
    this->device = device;
    this->bytesTotal = 0;

    connect(reply, SIGNAL(finished()),
                 this, SLOT(onFinished()));

    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
                 this, SLOT(onDownloadProgress(qint64, qint64)));

    connect(reply, SIGNAL(metaDataChanged()),
                 this, SLOT(onMetaDataChanged()));

    /*
    connect(reply, SIGNAL(uploadProgress(qint64, qint64)),
                 this, SLOT(onUploadProgress(qint64, qint64)));
    */
}

qint64 CustomDownload::max(qint64 num1, qint64 num2)
{
    return num1 > num2 ? num1 : num2;
}

void CustomDownload::onMetaDataChanged()
{
    if (isWaiting) {
        waitingTime = QDateTime::currentDateTime().toMSecsSinceEpoch()
                - waitingTime;
        isWaiting = false;

        downloadTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }
}

void CustomDownload::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesReceived == bytesTotal) {
        this->bytesTotal = bytesTotal;
    }
}

void CustomDownload::onFinished()
{
    // 触发自定义信号
    emit requestFinished(request.url().toString());

    // 其他数据计算
    requestEndTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    totalTime = requestEndTime - requestStartTime;

    downloadTime =
            QDateTime::currentDateTime().toMSecsSinceEpoch()
            - downloadTime;

    MonitorData *monitorData = new MonitorData();

    // TODO: 此部分参见
    // http://doc.qt.nokia.com/4.7-snapshot/qnetworkrequest.html
    // http://doc.qt.nokia.com/4.7-snapshot/qnetworkreply.html

    monitorData->StatusCode =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    // 状态码短语:OK/Not Found....
    monitorData->ReasonPhrase =
            reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    // 是否来自缓存内容
    monitorData->FromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();
    monitorData->RequestURL = request.url().toString();
    monitorData->ResponseSize = bytesTotal;
    monitorData->ResponseDuration = totalTime;
    monitorData->ResponseDNSLookupDuration = dnsLookupTime;
    monitorData->ResponseWaitingDuration =  waitingTime;
    monitorData->ResponseDownloadDuration = downloadTime;
    monitorData->RequestStartTime = requestStartTime;
    monitorData->RequestEndTime = requestEndTime;

    // GET or POST
    // TODO: 这里有6中状态 偶们把它们都记录下来
    //       参看 http://doc.qt.nokia.com/4.7/qnetworkaccessmanager.html
    monitorData->ResponseMethod =
            (reply->operation() == QNetworkAccessManager::GetOperation)
            ? "GET" : (reply->operation() == QNetworkAccessManager::PostOperation)
            ? "POST" : (reply->operation() == QNetworkAccessManager::HeadOperation)
            ? "HEAD" : (reply->operation() == QNetworkAccessManager::PutOperation)
            ? "PUT" : (reply->operation() == QNetworkAccessManager::DeleteOperation)
            ? "DELETE" : (reply->operation() == QNetworkAccessManager::CustomOperation)
            ? "CUSTOM": "Unknown";

    monitorData->UserAgent = request.rawHeader("User-Agent");
    monitorData->Accept = request.rawHeader("Accept");
    monitorData->Referer = request.rawHeader("Referer");

    QList<QPair<QByteArray, QByteArray> > rawHeaderPairs = reply->rawHeaderPairs();
    int c = rawHeaderPairs.size();

    for (int i = 0; i < c; ++i) {
        QString key = rawHeaderPairs.at(i).first;
        QString value = rawHeaderPairs.at(i).second;

        //qDebug()<< rawHeaderPairs.at(i).first << " : " << rawHeaderPairs.at(i).second;
        // 蛋疼的赋值……
        if ("Age" == key) {
            monitorData->Age = value;
        } else if ("Access-Control-Allow-Origin" == key) {
            monitorData->AccessControlAllowOrigin = value;
        } else if ("Cache-Control" == key) {
            monitorData->CacheControl = value;
        } else if ("Connection" == key) {
            monitorData->Connection = value;
        } else if ("Content-Type" == key) {
            monitorData->ContentType = value;
        } else if ("Content-Length" == key) {
            monitorData->ContentLength = value.toUInt();
        } else if ("Content-Encoding" == key) {
            monitorData->ContentEncoding = value;
        } else if ("Content-Language" == key) {
            monitorData->ContentLanguage = value;
        } else if ("Location" == key) {
            monitorData->Location = value;
        } else if ("Cookie" == key) {
            monitorData->Cookie = value;
        } else if ("Date" == key) {
            monitorData->Date = value;
        } else if ("ETag" == key) {
            monitorData->ETag= value;
        } else if ("Expires" == key) {
            monitorData->Expires = value;
        } else if ("If-Modified-Since" == key) {
            monitorData->IfModifiedSince= value;
        } else if ("Last-Modified" == key) {
            monitorData->LastModified = value;
        } else if ("Server" == key) {
            monitorData->Server = value;
        } else if ("Set-Cookie" == key) {
            monitorData->SetCookie = value;
        } else if ("P3P" == key) {
            monitorData->P3P = value;
        } else if ("Accept-Ranges" == key) {
            monitorData->AcceptRanges = value;
        } else if ("Vary" == key) {
            monitorData->Vary = value;
        } else if ("Transfer-Encoding" == key) {
            monitorData->TransferEncoding = value;
        } else if ("Via" == key) {
            monitorData->Via = value;
        } else if ("X-Via" == key) {
            monitorData->XVia = value;
        } else if ("X-DEBUG-IDC" == key) {
            monitorData->XDEBUGIDC = value;
        } else if ("X-Powered-By" == key) {
            monitorData->XPoweredBy = value;
        } else if ("X-Cache" == key) {
            monitorData->XCache = value;
        } else if ("X-Cache-Lookup" == key) {
            monitorData->XCacheLookup = value;
        } else if ("X-Cache-Varnish" == key) {
            monitorData->XCacheVarnish = value;
        } else if ("Powered-By-ChinaCache" == key) {
            monitorData->PoweredByChinaCache = value;
        } else if ("SINA-LB" == key) {
            monitorData->SINALB = value;
        } else {
            monitorData->other[key] = value;
        }
        QHostInfo info = QHostInfo::fromName(reply->url().host());
        if (!info.addresses().isEmpty()) {
            monitorData->ServerIPAddress = info.addresses().first().toString();
        }
        MonitorDataMap::getMonitorDataMap()->set(monitorData->RequestURL, monitorData);
    }

    QMap<QString, qint64> responseOtherData;
    responseOtherData["blocked"] =  0;
    responseOtherData["dns"] = monitorData->ResponseDNSLookupDuration;;
    responseOtherData["connect"] = 0;
    responseOtherData["send"] = 0;
    responseOtherData["wait"] =  monitorData->ResponseWaitingDuration;
    responseOtherData["receive"] = monitorData->ResponseDownloadDuration;
    responseOtherData["size"] = monitorData->ResponseSize;
    responseOtherData["start"] = monitorData->RequestStartTime;
    responseOtherData["end"] = monitorData->RequestEndTime;
    NetworkResources* nwr = NetworkResources::getInstance();
    nwr->addData(reply, device, responseOtherData);
}
