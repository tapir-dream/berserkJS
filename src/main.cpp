#include <QtGui/QApplication>
#include "mainwindow.h"
#include "commandparameters.h"


int main(int argc, char *argv[])
{
    QTextCodec *localCode = QTextCodec::codecForLocale();
    QTextCodec::setCodecForLocale(localCode);
    QTextCodec::setCodecForCStrings(localCode);
    QTextCodec::setCodecForTr(localCode);

    QApplication a(argc, argv);
    MainWindow w;
    CommandParameters commandParameters;

    // 显示帮助时 无视 --command 参数值 立即显示出窗口
    if (commandParameters.hasHelp()) {
        w.show();
        return a.exec();
    }

    // --command 参数没有时显示窗口
    if (!commandParameters.isCommandMode()) {
        w.show();
        return a.exec();
    }

    // 命令行模式并且没有开启help
    // 进行 start 和 script 脚本执行判断
    // 如果没有脚本可执行 则立即关闭自身
    if (commandParameters.isCommandMode() && !commandParameters.hasHelp()) {
        // 开启命令模式时 --script 参数值为空则立即退出
        if (commandParameters.hasScript() &&
            commandParameters.getParams()["script"] == "") {
            a.quit();
            return 0;
        }

        // 开启命令模式时 --script 参数值为空则立即退出
        if (!commandParameters.hasScript() ||
            !commandParameters.hasStart()) {
            a.quit();
            return 0;
        }
    }


    return a.exec();
}
