#ifndef MONITORDATA_H
#define MONITORDATA_H

/**
 * MonitorData 类
 * 监控数据的容器类
 * 类中实现一些基本判断
 * 如：是否有cookie，请求的文档类是什么等
 * @author Tapir
 * @date   2012-02-03
 */

#include <QObject>
#include <QDebug>
#include <QString>

class MonitorData: public QObject
{
    Q_OBJECT
private:

public:
    explicit MonitorData();

    // http 数据
    int StatusCode;
    QString ReasonPhrase;
    bool FromCache;
    QString ServerIPAddress;
    QString RequestURL;
    // 请求大小
    qint64 ResponseSize;
    // 请求实际执行时间
    qint64 ResponseDuration;
    // 请求等待时间
    qint64 ResponseWaitingDuration;
    // 数据下载时间
    qint64 ResponseDownloadDuration;
    // DNS 查找时间
    qint64 ResponseDNSLookupDuration;
    // 请求开始时间
    qint64 RequestStartTime;
    // 请求结束时间
    qint64 RequestEndTime;

    QString ResponseMethod;
    QString UserAgent;
    QString Accept;
    QString AcceptRanges;
    QString Referer;
    QString Age;
    QString AccessControlAllowOrigin;
    QString CacheControl;
    QString Connection;
    QString ContentType;
    uint ContentLength;
    QString ContentLanguage;
    QString ContentEncoding;
    QString Cookie;
    QString Date;
    QString ETag;
    QString Expires;
    QString IfModifiedSince;
    QString Location;
    QString LastModified;
    QString Server;
    QString SetCookie;
    QString P3P;
    QString Vary;
    QString TransferEncoding;
    QString Via;
    QString XVia;
    QString XDEBUGIDC;
    QString XPoweredBy;
    QString XCache;
    QString XCacheLookup;
    QString XCacheVarnish;
    QString PoweredByChinaCache;
    QString SINALB;
    QMap<QString, QString> other;

    bool hasKeepAlive();
    bool hasGZip();
    bool hasCookie();
    bool hasCache();
    bool hasExpires();
    bool isImgFile();
    bool isPng();
    bool isJpg();
    bool isGif();
    bool isIco();
    bool isSvg();
    bool isCssFile();
    bool isJsFile();
    bool isDocFile();
    bool isAudioFile();
    bool isVideoFile();
    bool isFontFile();
    bool isOtherFile();
    bool isHttp200();
    bool isHttp301();
    bool isHttp302();
    bool isHttp304();
    bool isHttp404();
    bool isFromCDN();
};

#endif // MONITORDATA_H
