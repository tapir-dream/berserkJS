#ifndef CUSTOMDOWNLOAD_H
#define CUSTOMDOWNLOAD_H

/**
 * CustomDownload 类
 * 用来汇总一个请求内所有的数据
 * 因为请求是异步的，需要这个容器来汇总内容
 * 由 NetworkAccessManager 类收集浏览发送的 HTTP 请求信息将发送到这个类中
 * @author Tapir
 * @date   2012-02-03
 */

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QDateTime>

class CustomDownload : public QObject
{
    Q_OBJECT
public:
    explicit CustomDownload(QNetworkReply* reply,
                            QNetworkRequest request,
                            qint64 dnsLookupTime,
                            qint64 requestStartTime);
private:
    QNetworkRequest request;
    QNetworkReply *reply;
    qint64 totalTime;
    qint64 waitingTime;
    qint64 dnsLookupTime;
    qint64 downloadTime;
    qint64 requestStartTime;
    qint64 requestEndTime;

    bool isWaiting;
    qint64 bytesTotal;

    qint64 max(qint64 num1, qint64 num2);


signals:
    void requestFinished(QString url);

public slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onFinished();
    void onMetaDataChanged();
};


#endif // CUSTOMDOWNLOAD_H
