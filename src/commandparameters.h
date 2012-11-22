#ifndef COMMANDPARAMETERS_H
#define COMMANDPARAMETERS_H

#include <QtGui/QApplication>
#include <QMap>

class CommandParameters
{
public:
    CommandParameters();
    bool isCommandMode();
    bool hasCache();
    bool hasScript();
    bool hasStart();
    bool hasHelp();
    bool hasVersion();
    QMap<QString, QString> params;
private:
    QMap<QString, QString> getParams();

};

#endif // COMMANDPARAMETERS_H
