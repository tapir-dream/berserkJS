#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commandparameters.h"
#include "scriptsignalfactory.h"

#include <QtGui/QApplication>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <QWidget>
#include <QHBoxLayout>
#include <QNetworkDiskCache>
#include <QDesktopServices>

MyWebView* MainWindow::webView;
MainWindow* MainWindow::window;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    helpUrl = "http://tapir-dream.github.com/berserkJS";
    scriptFunc = "App.loadScript(App.path + 'js/conf/init.js', function(err, func){func(App,App.webview)});";

    window = this;
    ui->setupUi(this);
    initLayout();
    initWebViewAttributes();
    initAppEngine();
    // 延时至 exec() 的消息循环启动，否则 close 、hide 等方法会失效。
    QTimer::singleShot(1, this, SLOT(initUserScript()));
}

void MainWindow::initUserScript()
{
    CommandParameters cmdParams;

    if (cmdParams.hasHelp()) {
        webView->load(QUrl(helpUrl));
        return;
    }

    if (cmdParams.hasScript()) {
        QString scriptFunc = script->readFile(getAppPath() +
                                              cmdParams.getParams()["script"],
                                              QTextCodec::codecForName("UTF-8"));

        if (script->getScriptEngine()->canEvaluate(scriptFunc)) {
            script->runScript(scriptFunc);
        } else {
            ui->outputScriptResults_txt->setPlainText(scriptFunc);
        }
    }

    if (cmdParams.hasStart()) {
        script->runScript(scriptFunc);
        return;
    }
}


void MainWindow::initLayout()
{
    // 布局自定义部件，并加入到窗口中
    webView = new MyWebView();
    webView->setGeometry(10,10,1024,600);
    this->ui->verticalLayout->addWidget(webView);
}

void MainWindow::initWebViewAttributes()
{
    QWebPage* page = webView->page();

    // 使用系统代理
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    // 设置本地缓存
    QNetworkDiskCache *diskCache = new QNetworkDiskCache(webView);
    QString location = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
    diskCache->setCacheDirectory(location);
    page->networkAccessManager()->setCache(diskCache);
    page->settings()->setMaximumPagesInCache(100);
}

void MainWindow::initAppEngine()
{
    // 为应用程序构造脚本对象
    ScriptBinding::webView = webView;
    script = new ScriptBinding();

    // 拿到信号工厂单例
    ScriptSignalFactory* ssf = new ScriptSignalFactory();
    ssf = ssf->instantiat();
    // 监听 信号工厂发出的信号
    connect(ssf, SIGNAL(consoleLogMessage(QString)), this, SLOT(onConsoleLogMessage(QString)));

    // 将应用程序脚本环境给webView对象，以便内部调用大App环境。
    webView->setAppScriptEngine(script);

    // 为脚本挂APP控制相关方法
    script->getRootSpace().setProperty("close",
        script->getScriptEngine()->newFunction(MainWindow::close),
        QScriptValue::ReadOnly);

    script->getRootSpace().setProperty("hide",
        script->getScriptEngine()->newFunction(MainWindow::hideOfScript),
        QScriptValue::ReadOnly);
    script->getRootSpace().setProperty("show",
        script->getScriptEngine()->newFunction(MainWindow::showOfScript),
        QScriptValue::ReadOnly);

    script->getRootSpace().setProperty("path",
        QScriptValue(getAppPath()), QScriptValue::ReadOnly);
    script->getRootSpace().setProperty("file",
        QScriptValue(getAppFileName()), QScriptValue::ReadOnly);
    script->getRootSpace().setProperty("args",
        getAppArguments(), QScriptValue::ReadOnly);

    QScriptValue webViewNamespace = script->getRootSpace().property("webview");
    webViewNamespace.setProperty("sendMouseEvent",
        script->getScriptEngine()->newFunction(MainWindow::sendEvent),
        QScriptValue::ReadOnly);

    // 做webview下方法的便捷引用
    script->getRootSpace().setProperty("netListener",
        webViewNamespace.property("netListener"),
        QScriptValue::ReadOnly);
    script->getRootSpace().setProperty("setTimeout",
        webViewNamespace.property("setTimeout"),
        QScriptValue::ReadOnly);
    script->getRootSpace().setProperty("clearTimeout",
        webViewNamespace.property("clearTimeout"),
        QScriptValue::ReadOnly);
    script->getRootSpace().setProperty("setInterval",
        webViewNamespace.property("setInterval"),
        QScriptValue::ReadOnly);
    script->getRootSpace().setProperty("clearInterval",
        webViewNamespace.property("clearInterval"),
        QScriptValue::ReadOnly);

    // 做 webview 相关方法的别名
    webViewNamespace.setProperty("on",
        webViewNamespace.property("addEventListener"));
    webViewNamespace.setProperty("un",
        webViewNamespace.property("removeEventListener"));
    webViewNamespace.setProperty("unAll",
        webViewNamespace.property("removeAllEventListener"));

    // 做APP下方法的便捷引用
    script->getGlobalObject().setProperty("alert",
        script->getRootSpace().property("alert"),
        QScriptValue::ReadOnly);
    script->getGlobalObject().setProperty("setTimeout",
        webViewNamespace.property("setTimeout"),
        QScriptValue::ReadOnly);
    script->getGlobalObject().setProperty("clearTimeout",
        webViewNamespace.property("clearTimeout"),
        QScriptValue::ReadOnly);
    script->getGlobalObject().setProperty("setInterval",
        webViewNamespace.property("setInterval"),
        QScriptValue::ReadOnly);
    script->getGlobalObject().setProperty("clearInterval",
        webViewNamespace.property("clearInterval"),
        QScriptValue::ReadOnly);

    // 构造内置对象基于 js 语法的简便包装
    // once 包装
    script->getScriptEngine()->evaluate("\
        App.webview.once = function(evtName, func) {\n\
            if (!evtName) return;\n\
            if (typeof func != 'function') return;\n\
            var cb = function() {\n\
               func.apply(App.webview, arguments);\n\
               App.webview.removeEventListener(evtName, cb);\n\
            };\n\
            App.webview.addEventListener(evtName, cb);\n\
        };\
   ");
}

QString MainWindow::getAppPath()
{
    return QApplication::applicationDirPath() + "/";
}

QString MainWindow::getAppFileName()
{
    return QApplication::applicationFilePath().split("/").last();
}

QScriptValue MainWindow::getAppArguments()
{
    QStringList args = QApplication::arguments();
    // 首参数是执行文件本身，为了避免混淆概念，这里将他移除。
    // 因此长度要是所有参数个数-1
    int len = args.size() - 1;
    if (len == 0)
       return QScriptValue::UndefinedValue;
    QScriptValue arr = script->getScriptEngine()->newArray(len);

    for (int i = 0; i < len; ++i) {
        arr.setProperty(i, QScriptValue(args.at(i+1)), QScriptValue::ReadOnly);
    }
    return arr;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_runScript_btn_clicked()
{
    ui->outputScriptResults_txt->setPlainText(
        script->runScript(
            ui->inputScriptCode_txt->toPlainText()
        ).toString() + '\n' +
        ui->outputScriptResults_txt->toPlainText()
    );
}

QScriptValue MainWindow::hideOfScript(QScriptContext *context, QScriptEngine *interpreter)
{
    window->hide();
    return QScriptValue(true);
}
QScriptValue MainWindow::showOfScript(QScriptContext *context, QScriptEngine *interpreter)
{
    window->show();
    return QScriptValue(true);
}

QScriptValue MainWindow::sendEvent(QScriptContext *context, QScriptEngine *interpreter)
{
    if (context->argumentCount() == 0)
        return QScriptValue::UndefinedValue;

    QScriptValue argPoint = context->argument(0);
    QScriptValue argEvt = context->argument(1);
    QScriptValue argKey = context->argument(2);

    if (!argPoint.isObject())
        return QScriptValue::UndefinedValue;


    // 构造事件触发位置默认值
    int x = 0;
    int y = 0;
    if (argPoint.property("x").isNumber())
        x = argPoint.property("x").toString().toInt();
    if (argPoint.property("y").isNumber())
        y = argPoint.property("y").toString().toInt();

    QPoint targetPoint(x, y);

    // 构造默认值鼠标事件类型默认值
    QMap<QString, QEvent::Type> mouseEventTypeMap;
    mouseEventTypeMap.insert("mousedown", QMouseEvent::MouseButtonPress);
    mouseEventTypeMap.insert("mouseup", QMouseEvent::MouseButtonRelease);
    mouseEventTypeMap.insert("mousemove", QMouseEvent::MouseMove);

    QMouseEvent::Type mouseEvtType = QMouseEvent::MouseButtonRelease;
    QString mouseEvtTypeStr = (!argEvt.isString()) ? "click"
                                                   : argEvt.toString().toLower();
    if (mouseEventTypeMap.contains(mouseEvtTypeStr))
        mouseEvtType = mouseEventTypeMap[mouseEvtTypeStr];

    // 构造辅助按键默认值
    QMap<QString, Qt::KeyboardModifier> keyTypeMap;
    keyTypeMap.insert("shift", Qt::ShiftModifier);
    keyTypeMap.insert("ctrl", Qt::ShiftModifier);
    keyTypeMap.insert("alt", Qt::ShiftModifier);

    Qt::KeyboardModifier keyType = Qt::NoModifier;
    QString keyTypeStr = argKey.toString().toLower();
    if (keyTypeMap.contains(keyTypeStr))
        keyType = keyTypeMap[keyTypeStr];

    // 修复鼠标移动事件的按键值，API原文说明：
    // If the event type is MouseMove,
    // the appropriate button for this event is Qt::NoButton.
    QMouseEvent* event;
    if (mouseEvtType == QMouseEvent::MouseMove) {
        event = new QMouseEvent(mouseEvtType, targetPoint,
                                Qt::NoButton, Qt::NoButton, keyType);
    } else {
        event = new QMouseEvent(mouseEvtType, targetPoint,
                                Qt::LeftButton, Qt::LeftButton, keyType);
    }

    // 对click操作做特殊处理，他是mousdown与mouseup操作的结合体
    if (mouseEvtTypeStr == "click") {
        QMouseEvent* clickEvt1 = new QMouseEvent(QMouseEvent::MouseButtonPress, targetPoint,
                                Qt::LeftButton, Qt::LeftButton, keyType);
        QMouseEvent* clickEvt2 = new QMouseEvent(QMouseEvent::MouseButtonRelease, targetPoint,
                                Qt::LeftButton, Qt::LeftButton, keyType);
        bool b = QApplication::sendEvent(webView, clickEvt1) &&
                 QApplication::sendEvent(webView, clickEvt2);

        delete clickEvt1;
        delete clickEvt2;
        return b;
    }

    bool b = QApplication::sendEvent(webView, event);
    delete event;
    return b;
}

QScriptValue MainWindow::close(QScriptContext *context, QScriptEngine *interpreter)
{
    QApplication::quit();
    return QScriptValue::UndefinedValue;
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->modifiers() == Qt::ControlModifier
       && e->key() == Qt::Key_Return) {
       on_runScript_btn_clicked();
       return;
    }
    if (e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)
       && e->key() == Qt::Key_Return) {
        on_clearLog_btn_clicked();
    }
    if ((e->key() == Qt::Key_F12) ||
        (e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)
            && e->key() == Qt::Key_J))  {
        viewChange();
    }

}

void MainWindow::on_clearLog_btn_clicked()
{
    ui->outputLogResults_txt->setPlainText("");
    ui->outputScriptResults_txt->setPlainText("");
}

void MainWindow::onConsoleLogMessage(QString str)
{
    ui->outputLogResults_txt
      ->setPlainText(str + "\n------------------------------\n" +
                     ui->outputLogResults_txt->toPlainText());
}



void MainWindow::viewChange()
{
    if (!ui->inputScriptCode_txt->isHidden()) {
        ui->inputScriptCode_txt->hide();
        ui->outputLogResults_txt->hide();
        ui->outputScriptResults_txt->hide();
        ui->runScript_btn->hide();
        ui->clearLog_btn->hide();
    } else {
        ui->inputScriptCode_txt->show();
        ui->outputLogResults_txt->show();
        ui->outputScriptResults_txt->show();
        ui->runScript_btn->show();
        ui->clearLog_btn->show();
    }
}
