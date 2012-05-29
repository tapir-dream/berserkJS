#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore/QDebug>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtScript>
#include <QStringList>
#include <QWebInspector>
#include <QtUiTools/QtUiTools>
#include <QtUiTools/QUiLoader>
#include <QKeyEvent>

#include "networkaccessmanager.h"
#include "monitordatamap.h"
#include "monitordata.h"
#include "selector.h"
#include "scriptbinding.h"
#include "mywebview.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

    static QScriptValue close(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue sendEvent(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue hideOfScript(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue showOfScript(QScriptContext *context, QScriptEngine *interpreter);

    static MyWebView* webView;
    static MainWindow* window;

private:
    Ui::MainWindow *ui;
    ScriptBinding* script;

    QString getAppPath();
    QString getAppFileName();
    QScriptValue getAppArguments();

    void initLayout();
    void initWebViewAttributes();
    void initAppEngine();
    void keyPressEvent(QKeyEvent *e);


public slots:

private slots:
    void initUserScript();
    void onConsoleLogMessage(QString str);

    //使用自动槽
    void on_runScript_btn_clicked();
    void on_clearLog_btn_clicked();

};

#endif // MAINWINDOW_H
