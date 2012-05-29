#include "paramoption.h"



paramOption::paramOption(QObject *parent) :
    QObject(parent)
{
}

void paramOption::set(int key, QString value)
{
    this->map[QString(key)] = value;
}

bool paramOption::hasOption(QString key)
{
    if (this->map[QString(key)]) {
        return false;
    }else{
        return true;
    }
}

