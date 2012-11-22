#include <QtGui/QApplication>
#include "mainwindow.h"
#include "commandparameters.h"
#include "consts.h"


int main(int argc, char *argv[])
{
    QTextCodec *localCode = QTextCodec::codecForLocale();
    QTextCodec::setCodecForLocale(localCode);
    QTextCodec::setCodecForCStrings(localCode);
    QTextCodec::setCodecForTr(localCode);

    QApplication a(argc, argv);
    CommandParameters commandParameters;
    MainWindow w;

    // 显示版本号时无视其它参数内容
    if (commandParameters.hasVersion()) {
        printf(VERSION_STRING);
        return 0;
    }

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
            commandParameters.params["script"] == "") {
            a.quit();
            return 0;
        }

        // 开启命令模式时 --script 参数值为空则立即退出
        if (!commandParameters.hasScript() &&
            !commandParameters.hasStart()) {
            a.quit();
            return 0;
        }
    }

    // 用来修补sendEvent在不显示窗口时失效问题
    // 先最小化显示再隐藏
    // show窗口之后才能开始接收GUI归属的消息
    w.showMinimized();
    w.hide();

    return a.exec();
}
