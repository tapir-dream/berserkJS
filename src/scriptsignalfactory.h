#ifndef SCRIPTSIGNALFACTORY_H
#define SCRIPTSIGNALFACTORY_H

#include <QObject>

class ScriptSignalFactory : public QObject
{
    Q_OBJECT
public:
    // 区分信号类型
    enum signalType {
        consoleLog
    };

    ScriptSignalFactory* instantiat();
    void fire(signalType type, QString str);

private:
    static ScriptSignalFactory* scriptSignalFactory;

signals:
    // console.log 的自定义信号
    void consoleLogMessage(QString str);

public slots:
    
};

#endif // SCRIPTSIGNALFACTORY_H
