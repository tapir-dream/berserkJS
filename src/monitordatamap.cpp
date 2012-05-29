#include "monitordatamap.h"

MonitorDataMap* MonitorDataMap::monitorDataMap = 0;

MonitorDataMap* MonitorDataMap::getMonitorDataMap()
{
    if(monitorDataMap == 0) {
          monitorDataMap = new MonitorDataMap();
    }
    return monitorDataMap;
}

void MonitorDataMap::set(QString url, MonitorData* monitorData)
{
    dataMap.insert(url, monitorData);
}

void MonitorDataMap::clear()
{
    QList<QString> keys = dataMap.keys();
    int c = dataMap.size();
    for (int i = 0; i < c; ++i) {
        delete dataMap[keys.at(i)];
    }
    dataMap.clear();
}

QMap<QString, MonitorData*> MonitorDataMap::getData()
{
    return dataMap;
}

void MonitorDataMap::setData(QMap<QString, MonitorData*> dataMap)
{
    this->dataMap = dataMap;
}

