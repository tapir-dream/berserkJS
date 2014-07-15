#ifndef NETWORKRESOURCES_H
#define NETWORKRESOURCES_H

/**
 * NetworkResources 类
 * 网络请求数据集合
 * @author Tapir
 * @date   2014-07-10
 */


#include <QObject>
#include <QMap>
#include <QString>
#include <QList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QIODevice>
#include <QNetworkCookieJar>

class NetworkResources
{


public:
    typedef struct {
        QMap<QString, QString> headers;
        QVariantMap base;
    } requestData;

    typedef struct {
        QMap<QString, QString> headers;
        QVariantMap base;
    } responseData;

    typedef struct {
        requestData request;
        responseData response;
        QMap<QString, qint64> timings;
    } resource;
    static NetworkResources* getInstance();
    void addData(const QNetworkReply* reply, QIODevice *device,
                 const QMap<QString, qint64> responseOtherData);
    void clear();
    QList<resource> getData();

private:
    static NetworkResources* networkResources;
    QList<resource> resourceList;

};

#endif // NETWORKRESOURCES_H
