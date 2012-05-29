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

    if (!commandParameters.isCommandMode()) {
        w.show();
    }
    return a.exec();
}
