#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

/**
 * NetworkAccessManager 类
 * 用来覆盖原有 QtWebkit 中的 QNetworkAccessManager 实例
 * 使用新的 NetworkAccessManager 类实例收集浏览发送的 HTTP 请求信息
 * @author Tapir
 * @date   2012-02-03
 */

#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QList>
#include <QDebug>

class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    explicit NetworkAccessManager(QNetworkAccessManager *manager, QObject *parent);

    QNetworkReply* createRequest(QNetworkAccessManager::Operation operation,
                       const QNetworkRequest &request, QIODevice *device);

    static bool isListener;

private:
    QNetworkRequest request;

signals:
    void requestFinished(QString url);
    void requestStart(QString url);

public slots:
    void onRequestFinished(QString url);
};

#endif // NETWORKACCESSMANAGER_H
