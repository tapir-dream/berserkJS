#include "scriptsignalfactory.h"

ScriptSignalFactory* ScriptSignalFactory::scriptSignalFactory = 0;

ScriptSignalFactory* ScriptSignalFactory::instantiat()
{
    if (ScriptSignalFactory::scriptSignalFactory == 0) {
        ScriptSignalFactory::scriptSignalFactory = new ScriptSignalFactory();
    }
    return ScriptSignalFactory::scriptSignalFactory;
}

void ScriptSignalFactory::fire(signalType type, QString str)
{
    // TODO: 可以继续扩充枚举类型与信号触发的对应
    switch(type) {
        case consoleLog:
            emit consoleLogMessage(str);
            break;
    }
}

