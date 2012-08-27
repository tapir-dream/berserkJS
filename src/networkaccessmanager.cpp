#include "networkaccessmanager.h"
#include "customdownload.h"

bool NetworkAccessManager::isListener = false;
NetworkAccessManager::NetworkAccessManager(QNetworkAccessManager *manager, QObject *parent)
    : QNetworkAccessManager(parent)
{
    setCache(manager->cache());
    setCookieJar(manager->cookieJar());
    setProxy(manager->proxy());
    setProxyFactory(manager->proxyFactory());
}

QNetworkReply* NetworkAccessManager::createRequest(QNetworkAccessManager::Operation operation,
    const QNetworkRequest &request, QIODevice *device)
{
    qint64 time;
    if (isListener) {
        time = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    QNetworkReply *reply = QNetworkAccessManager::createRequest(operation, request, device);
    reply->ignoreSslErrors();
    if (isListener) {
       // 触发请求开始的自定义信号
       emit requestStart(request.url().toString());

       // 不清除这个指针
       // TODO: 该类由 webview 接管处理
       CustomDownload* customDownload = new CustomDownload(reply, request,
                           QDateTime::currentDateTime().toMSecsSinceEpoch() - time,
                           time);

       connect(customDownload, SIGNAL(requestFinished(QString)),
               this, SLOT(onRequestFinished(QString)));
    }

    return reply;
}

void NetworkAccessManager::onRequestFinished(QString url)
{
    // 接力触发请求结束的自定义信号
    emit requestFinished(url);
}

