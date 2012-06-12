#ifndef GETCPUPERCENTAGE_H
#define GETCPUPERCENTAGE_H
#include <QDebug>

class GetCPUPercentage
{
public:
    GetCPUPercentage();
    ~GetCPUPercentage();
    double get();  // 得到CPU占用率
};
#endif // GETCPUPERCENTAGE_H
