#ifndef COMMANDPARAMETERS_H
#define COMMANDPARAMETERS_H

#include <QtGui/QApplication>
#include <QMap>

class CommandParameters
{
public:
    CommandParameters();
    QMap<QString, QString> getParams();
    bool isCommandMode();
    bool hasScript();
    bool hasStart();
    bool hasHelp();
    bool hasVersion();
};

#endif // COMMANDPARAMETERS_H
