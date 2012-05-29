#ifndef SELECTOR_H
#define SELECTOR_H
#include <QObject>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QString>
#include "monitordata.h"
#include "monitordatamap.h"

/**
 * Selector 类
 * 该类用来选择指定类型的未过滤数据,返回更小的数据集合。
 *
 * @author Tapir
 * @date   2012-02-13
 */

class Selector: public QObject
{
    Q_OBJECT
private:
    // 区分大类型
    enum selectType {
        IMG,
        PNG,
        GIF,
        ICO,
        JPG,
        SVG,
        Doc,
        CSS,
        JS,
        TotalTimeout,
        WaitTimeout,
        DownloadTimeout,
        DNSLookupTimeout,
        SizeOut,
        Cookie,
        NoneCache,
        NoneGZip,
        NoneCND,
        FromCDN,
        Http200,
        Http301,
        Http302,
        Http304,
        Http404
    };
    MonitorDataMap* monitorDataMapHandle;
    QMap<QString, MonitorData*> selectedMonitorDataMap;
    void addToSelectedMonitorDataMap(selectType type, qint32 value = 0);
public:

    Selector();
    void clear();
    void selectedIMG();
    void selectedPNG();
    void selectedGIF();
    void selectedICO();
    void selectedJPG();
    void selectedSVG();
    void selectedDoc();
    void selectedCSS();
    void selectedJS();
    void selectedTotalTimeout(qint32 value);
    void selectedWaitTimeout(qint32 value);
    void selectedDownloadTimeout(qint32 value);
    void selectedDNSLookupTimeout(qint32 value);
    void selectedSizeOut(qint32 value);
    void selectedCookie();
    void selectedNoneGZip();
    void selectedNoneCache();
    void selectedNoneCDN();
    void selectedHttp200();
    void selectedHttp301();
    void selectedHttp302();
    void selectedHttp304();
    void selectedHttp404();
    void selectedFromCDN();
    QMap<QString, MonitorData*> get();
};

#endif // SELECTOR_H
