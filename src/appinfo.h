#ifndef APPINFO_H
#define APPINFO_H
#include <QDebug>

class AppInfo
{
public:
    AppInfo();
    ~AppInfo();
    double getCPU();  // 得到CPU占用率
    double getMemory();  // 得到内存占用率
};

#endif // APPINFO_H
