#ifndef MONITORDATAMAP_H
#define MONITORDATAMAP_H

/**
 * MonitorDataMap 类
 * 监控数据的容器类的集合
 * 所有由 QtWebkit 收集到的 MonitorData 类实例将被放置在这里
 * @author Tapir
 * @date   2012-02-03
 */


#include "monitordata.h"
#include <QObject>
#include <QMap>
#include <QString>
#include <cstddef>

class MonitorDataMap: public QObject
{
    Q_OBJECT
private:
    static MonitorDataMap *monitorDataMap;
    QMap<QString, MonitorData*> dataMap;

public:
    static MonitorDataMap* getMonitorDataMap();
    QMap<QString, MonitorData*> getData();
    void set(QString url, MonitorData* monitorData);
    void setData(QMap<QString, MonitorData*> dataMap);
    void clear();
};

#endif // MONITORDATAMAP_H
