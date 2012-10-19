#include "scriptbinding.h"
#include "commandparameters.h"
#include "scriptsignalfactory.h"
#include "consts.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

// 静态非int常量不可以在类体内初始化……
const QString ScriptBinding::ROOT = "App";
const QString ScriptBinding::EXPAND_METHOD = "expand";
const QString ScriptBinding::CONSOLE = "console";
const QString ScriptBinding::OPEN_FILE_ERROR = "Error: Cannot open file ";

/* Selector 类需要暴露的方法包裹 */
Selector* ScriptBinding::selector = 0;
QWebView* ScriptBinding::webView = 0;

AppInfo* ScriptBinding::appInfo = new AppInfo();
FileSystemWatcher* ScriptBinding::fileSystemWatcher;

QMap<QString, qint64> ScriptBinding::timeMap;

ScriptBinding::ScriptBinding()
{
    // 初始化文件监控对象
    fileSystemWatcher = new FileSystemWatcher();
    // 选择器
    selector = new Selector();

    initScriptEngine();
    initRootSpace();
    initNativeMethodToRootSpace();
    initConsoleSpace();

    //暂时不用的
    //initExpandMethodToRootSpace();
}

ScriptBinding::~ScriptBinding()
{
    delete selector;
    delete fileSystemWatcher;
    delete engine;
    delete appInfo;
}

void ScriptBinding::initScriptEngine()
{
    engine = new QScriptEngine();
    globalObject = engine->globalObject();
}

// 构造 Root 空间
void ScriptBinding::initRootSpace()
{
    QScriptValue obj = engine->newObject();
    globalObject.setProperty(ROOT, obj);
}

QScriptValue ScriptBinding::getRootSpace()
{
    return globalObject.property(ROOT);
}

void ScriptBinding::initConsoleSpace()
{
    QScriptValue obj = engine->newObject();

    QScriptValue nativeMathod;
    nativeMathod = engine->newFunction(ScriptBinding::consoleLog);
    obj.setProperty("log", nativeMathod);
    nativeMathod = engine->newFunction(ScriptBinding::consoleTime);
    obj.setProperty("time", nativeMathod);
    nativeMathod = engine->newFunction(ScriptBinding::consoleTimeEnd);
    obj.setProperty("timeEnd", nativeMathod);
    nativeMathod = engine->newFunction(ScriptBinding::consoleDir);
    obj.setProperty("dir", nativeMathod);

    globalObject.setProperty(CONSOLE, obj);
}

QScriptValue ScriptBinding::getGlobalObject()
{
    return globalObject;
}

QScriptEngine* ScriptBinding::getScriptEngine()
{
    return engine;
}

QTextCodec* ScriptBinding::getCodec(QString charset)
{
    // 解决页面源数据编码问题
    QMap<QString, QString> codecs;
    codecs.insert("UTF-8", "UTF-8");
    codecs.insert("UTF-16", "UTF-16");
    codecs.insert("GB18030-0", "GB2312");
    codecs.insert("GBK", "GB2312");
    codecs.insert("GB2312", "GB2312");
    codecs.insert("ISO-8859-1", "ISO-8859-1");
    codecs.insert("BIG5", "BIG5");
    codecs.insert("BIG5-HKSCS", "BIG5");

    charset = charset.toUpper();
    if (codecs.contains(charset)) {
        charset = codecs[charset];
    } else {
        charset = codecs["UTF-8"];
    }

    return QTextCodec::codecForName(charset.toAscii());
}

QString ScriptBinding::readFile(QString fileName, QTextCodec* charset)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return OPEN_FILE_ERROR + qPrintable(fileName)
                 + ": " + qPrintable(file.errorString());
    }
    QTextStream in(&file);
    in.setCodec(charset);
    QString code = in.readAll();
    file.close();
    return code;
}

/*
 * loadScript 方法
 * @param string  js路径(支持本地路径以及http与https协议下远程js获取)
 * @param function 调用函数
 * @return
 *   如果第二个参数不存在, load 失败，返回 false 。
 *   如果第二个参数不存在, load 成功，则返回 true 。
 *   如果第二参数存在，则立即调用内部函数，返回执行结果。
 *      load的内容将以回调函数参数传入：
 *          @param bool err 表明是否正常读取内容
 *          @param function callback 回一个load函数引用
 */
QScriptValue ScriptBinding::loadScript(QScriptContext *context, QScriptEngine *interpreter)
{
    QScriptValue path = context->argument(0);
    QScriptValue scriptFunc = context->argument(1);

    if (context->argumentCount() == 0)
        return QScriptValue(false);

    if (!path.isString())
        return QScriptValue(false);


    QString pathStr = path.toString().toLower().trimmed();
    QString content = "";
    bool err = false;

    // 如果是 HTTP 、 HTTPS 则尝试从远端获取源码
    if (pathStr.indexOf("http://") == 0 || pathStr.indexOf("https://") == 0 ) {
        QNetworkReply* reply;
        QNetworkAccessManager* manager = new QNetworkAccessManager();
        reply = manager->get(QNetworkRequest(QUrl(pathStr)));

        QEventLoop eventLoop;
        connect(manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
        eventLoop.exec();

        QByteArray responseData;
        responseData = reply->readAll();

        // 通过 Content-Type 来嗅探字节流编码
        // 默认为 utf-8 编码
        QString charset = QString(reply->rawHeader("Content-Type")).toLower();
        QRegExp charsetRegExp("charset=([\\w-]+)\\b");
        int pos = charset.indexOf(charsetRegExp);
        if (pos > 0) {
            if (charsetRegExp.cap().size() < 2) {
                charset = "utf-8";
            } else {
                charset = charsetRegExp.cap(1);
            }

        } else {
            charset = "utf-8";
        }

        QTextStream stream(responseData);
        stream.setCodec(getCodec(charset));
        content = QString(stream.readAll());

    } else {
        // 如果是本地文件则尝试读取 并且需要判断扩展名
        if (!pathStr.lastIndexOf(".js") + 3 == path.toString().length()) {
            return QScriptValue(false);
        }

        content = ScriptBinding::readFile(path.toString(), getCodec("UTF-8"));
        err = (content.indexOf(OPEN_FILE_ERROR) == 0);

        if (err) {
            return QScriptValue(false);
        }
    }

    if (!interpreter->canEvaluate(content))
        return QScriptValue(false);

    QScriptValue scope = context->activationObject().scope();
    QScriptValue thisObject = context->thisObject();

    // 如果没有第二个参数 则说明代码可以直接执行
    if (!scriptFunc.isFunction()) {
        // 直接执行代码
        interpreter->evaluate(content);
        // 如果遇到错误就用console.log 显示，并且return false;
        if (interpreter->hasUncaughtException()) {
            QString scriptErr = "Uncaught exception at line "
                 + QString::number(interpreter->uncaughtExceptionLineNumber()) + ": "
                 + interpreter->uncaughtException().toString()
                 + "; Backtrace: "
                 + interpreter->uncaughtExceptionBacktrace().join(", ");
            interpreter->evaluate("console.log('" + scriptErr + "')");
            return  QScriptValue(false);
        }
        return QScriptValue(true);
    }

    if (scriptFunc.isFunction()) {
        QScriptValue contentFunc = interpreter->evaluate(content);
        scriptFunc.setScope(scope);
        return scriptFunc.call(thisObject, QScriptValueList() << (err ? true : false) << contentFunc);
    }

    return QScriptValue(false);
}

QScriptValue ScriptBinding::runScript(const QString code)
{
    try {
        QScriptValue scriptValue = engine->evaluate(code);
        if (engine->hasUncaughtException()) {
              return "Uncaught exception at line "
                     + QString::number(engine->uncaughtExceptionLineNumber()) + ": "
                     + engine->uncaughtException().toString()
                     + "Backtrace: "
                     + engine->uncaughtExceptionBacktrace().join(", ");

        }
        return scriptValue;
    } catch(std::exception& e) {
        return QScriptValue::UndefinedValue;
    }
}

QScriptValue ScriptBinding::readFile(QScriptContext *context, QScriptEngine *interpreter)
{
    QScriptValue path = context->argument(0);
    QScriptValue charset = context->argument(1);
    QString content;

    if (context->argumentCount() == 0)
        return QScriptValue(false);

    if (!path.isString())
        return QScriptValue(false);

    QString codec = "";
    if (charset.isString()) {
        codec = charset.toString();
    }

    content = ScriptBinding::readFile(path.toString(), getCodec(codec));

    bool err = (content.indexOf(OPEN_FILE_ERROR) == 0);

    if (err)
        return QScriptValue(false);

    return QScriptValue(content);
}

QScriptValue ScriptBinding::writeFile(QScriptContext *context, QScriptEngine *interpreter)
{
    QScriptValue path = context->argument(0);
    QScriptValue text = context->argument(1);
    QScriptValue charset = context->argument(2);
    QScriptValue appendMode = context->argument(3);

    if (context->argumentCount() == 0)
        return QScriptValue(false);

    if (!path.isString())
        return QScriptValue(false);

    if (!text.isString()) {
        text = text.toString();
    }

    bool isAppend = appendMode.toBool();

    QString codec = "";
    if (charset.isString()) {
        codec = charset.toString();
    }

    QFile file(path.toString());

    if (isAppend) {
        if (!file.open(QIODevice::Append))
             return QScriptValue(false);
    } else {
        if (!file.open(QIODevice::WriteOnly))
            return QScriptValue(false);
    }

    QTextStream stream(&file);
    stream.setCodec(getCodec(codec));
    stream << text.toString();
    file.close();

    return QScriptValue(true);
}

QScriptValue ScriptBinding::toWrapper(QObject *object)
{
    return engine->newQObject(object, QScriptEngine::QtOwnership,
                                QScriptEngine::PreferExistingWrapperObject
                                | QScriptEngine::ExcludeSuperClassContents);
}

QScriptValue ScriptBinding::wrapperHttpDataArray(QMap<QString, MonitorData*> map, QScriptEngine* engine)
{
    QList<QString> keys = map.keys();
    int len =keys.size();

    // 建立容器，每条 network 请求是为一个object，包含key->value数据
    // 每个 network 对象容器最终被放置在数组内

    QScriptValue array = engine->newArray(len);
    for (int i = 0; i < len; ++i) {
        QScriptValue obj = engine->newObject();

        // 只读的源数据部分
        obj.setProperty("StatusCode", QScriptValue(map[keys.at(i)]->StatusCode), QScriptValue::ReadOnly);
        obj.setProperty("ReasonPhrase", QScriptValue(map[keys.at(i)]->ReasonPhrase), QScriptValue::ReadOnly);
        obj.setProperty("FromCache", QScriptValue(map[keys.at(i)]->FromCache), QScriptValue::ReadOnly);
        obj.setProperty("url", QScriptValue(map[keys.at(i)]->RequestURL), QScriptValue::ReadOnly);
        obj.setProperty("RequestStartTime", QScriptValue(QString::number(map[keys.at(i)]->RequestStartTime).toDouble()), QScriptValue::ReadOnly);
        obj.setProperty("RequestEndTime", QScriptValue(QString::number(map[keys.at(i)]->RequestEndTime).toDouble()), QScriptValue::ReadOnly);
        obj.setProperty("ResponseSize", QScriptValue(QString::number(map[keys.at(i)]->ResponseSize).toDouble()), QScriptValue::ReadOnly);
        obj.setProperty("ResponseDuration", QScriptValue(QString::number(map[keys.at(i)]->ResponseDuration).toDouble()), QScriptValue::ReadOnly);
        obj.setProperty("ResponseWaitingDuration", QScriptValue(QString::number(map[keys.at(i)]->ResponseWaitingDuration).toDouble()), QScriptValue::ReadOnly);
        obj.setProperty("ResponseDownloadDuration", QScriptValue(QString::number(map[keys.at(i)]->ResponseDownloadDuration).toDouble()), QScriptValue::ReadOnly);
        obj.setProperty("ResponseDNSLookupDuration", QScriptValue(QString::number(map[keys.at(i)]->ResponseDNSLookupDuration).toDouble()), QScriptValue::ReadOnly);
        obj.setProperty("ResponseMethod", QScriptValue(map[keys.at(i)]->ResponseMethod), QScriptValue::ReadOnly);
        obj.setProperty("UserAgent", QScriptValue(map[keys.at(i)]->UserAgent), QScriptValue::ReadOnly);
        obj.setProperty("Accept", QScriptValue(map[keys.at(i)]->Accept), QScriptValue::ReadOnly);
        obj.setProperty("Referer", QScriptValue(map[keys.at(i)]->Referer), QScriptValue::ReadOnly);
        obj.setProperty("AcceptRanges", QScriptValue(map[keys.at(i)]->AcceptRanges), QScriptValue::ReadOnly);
        obj.setProperty("Age", QScriptValue(map[keys.at(i)]->Age), QScriptValue::ReadOnly);
        obj.setProperty("AccessControlAllowOrigin", QScriptValue(map[keys.at(i)]->AccessControlAllowOrigin), QScriptValue::ReadOnly);
        obj.setProperty("CacheControl", QScriptValue(map[keys.at(i)]->CacheControl), QScriptValue::ReadOnly);
        obj.setProperty("Connection", QScriptValue(map[keys.at(i)]->Connection), QScriptValue::ReadOnly);
        obj.setProperty("ContentType", QScriptValue(map[keys.at(i)]->ContentType), QScriptValue::ReadOnly);
        obj.setProperty("ContentLength", QScriptValue(map[keys.at(i)]->ContentLength), QScriptValue::ReadOnly);
        obj.setProperty("ContentEncoding", QScriptValue(map[keys.at(i)]->ContentEncoding), QScriptValue::ReadOnly);
        obj.setProperty("ContentLanguange", QScriptValue(map[keys.at(i)]->ContentLanguage), QScriptValue::ReadOnly);
        obj.setProperty("Cookie", QScriptValue(map[keys.at(i)]->Cookie), QScriptValue::ReadOnly);
        obj.setProperty("Date", QScriptValue(map[keys.at(i)]->Date), QScriptValue::ReadOnly);
        obj.setProperty("ETag", QScriptValue(map[keys.at(i)]->ETag), QScriptValue::ReadOnly);
        obj.setProperty("Expires", QScriptValue(map[keys.at(i)]->Expires), QScriptValue::ReadOnly);
        obj.setProperty("IfModifiedSince", QScriptValue(map[keys.at(i)]->IfModifiedSince), QScriptValue::ReadOnly);
        obj.setProperty("LastModified", QScriptValue(map[keys.at(i)]->LastModified), QScriptValue::ReadOnly);
        obj.setProperty("Location", QScriptValue(map[keys.at(i)]->Location), QScriptValue::ReadOnly);
        obj.setProperty("Server", QScriptValue(map[keys.at(i)]->Server), QScriptValue::ReadOnly);
        obj.setProperty("SetCookie", QScriptValue(map[keys.at(i)]->SetCookie), QScriptValue::ReadOnly);
        obj.setProperty("P3P", QScriptValue(map[keys.at(i)]->P3P), QScriptValue::ReadOnly);
        obj.setProperty("Vary", QScriptValue(map[keys.at(i)]->Vary), QScriptValue::ReadOnly);
        obj.setProperty("TransferEncoding", QScriptValue(map[keys.at(i)]->TransferEncoding), QScriptValue::ReadOnly);
        obj.setProperty("Via", QScriptValue(map[keys.at(i)]->Via), QScriptValue::ReadOnly);
        obj.setProperty("XVia", QScriptValue(map[keys.at(i)]->XVia), QScriptValue::ReadOnly);
        obj.setProperty("XDEBUGIDC", QScriptValue(map[keys.at(i)]->XDEBUGIDC), QScriptValue::ReadOnly);
        obj.setProperty("XPoweredBy", QScriptValue(map[keys.at(i)]->XPoweredBy), QScriptValue::ReadOnly);
        obj.setProperty("XCache", QScriptValue(map[keys.at(i)]->XCache), QScriptValue::ReadOnly);
        obj.setProperty("XCacheLookup", QScriptValue(map[keys.at(i)]->XCacheLookup), QScriptValue::ReadOnly);
        obj.setProperty("XCacheVarnish", QScriptValue(map[keys.at(i)]->XCacheVarnish), QScriptValue::ReadOnly);
        obj.setProperty("PoweredByChinaCache", QScriptValue(map[keys.at(i)]->PoweredByChinaCache), QScriptValue::ReadOnly);
        obj.setProperty("SINALB", QScriptValue(map[keys.at(i)]->SINALB), QScriptValue::ReadOnly);

        // 遍历输出未知应答头（自定义私有头）
        QMap<QString, QString> other = map[keys.at(i)]->other;
        int otherHeadSize = other.size();
        if (otherHeadSize > 0) {
            for (int j = 0; j < otherHeadSize; ++j) {
                QList<QString> otherKeys = other.keys();
                obj.setProperty(otherKeys.at(j),
                                QScriptValue(other[otherKeys.at(j)]),
                                QScriptValue::ReadOnly);
            }
        }

        // 由于无法从监听reply中读到请求的字节数据，只能将url放到浏览器中 new Image
        // 此时浏览器已经加载完成图片，可以立即得到宽高值。
        // 无奈…… 阿弥陀佛……
        if (map[keys.at(i)]->isImgFile()) {
            QStringList rect = ScriptBinding::webView->page()->mainFrame()->evaluateJavaScript(
                "(function() {var img = new Image(); img.src = '" +
                map[keys.at(i)]->RequestURL +
                "'; return img.width + '|' + img.height;})();"
             ).toString().split("|");
             obj.setProperty("width", QScriptValue(rect.at(0).toInt()), QScriptValue::ReadOnly);
             obj.setProperty("height", QScriptValue(rect.at(0).toInt()), QScriptValue::ReadOnly);
        } else {
            obj.setProperty("width", QScriptValue(-1), QScriptValue::ReadOnly);
            obj.setProperty("height", QScriptValue(-1), QScriptValue::ReadOnly);
        }

        // 由内核提供的一些辅助数据可被更改
        obj.setProperty("hasKeepAlive", QScriptValue(map[keys.at(i)]->hasKeepAlive()));
        obj.setProperty("hasGZip", QScriptValue(map[keys.at(i)]->hasGZip()));
        obj.setProperty("hasCookie", QScriptValue(map[keys.at(i)]->hasCookie()));
        obj.setProperty("hasCache", QScriptValue(map[keys.at(i)]->hasCache()));
        obj.setProperty("hasExpires", QScriptValue(map[keys.at(i)]->hasExpires()));
        obj.setProperty("isFromCDN", QScriptValue(map[keys.at(i)]->isFromCDN()));
        obj.setProperty("isImgFile", QScriptValue(map[keys.at(i)]->isImgFile()));
        obj.setProperty("isPng", QScriptValue(map[keys.at(i)]->isPng()));
        obj.setProperty("isJpg", QScriptValue(map[keys.at(i)]->isJpg()));
        obj.setProperty("isGif", QScriptValue(map[keys.at(i)]->isGif()));
        obj.setProperty("isIco", QScriptValue(map[keys.at(i)]->isIco()));
        obj.setProperty("isSvg", QScriptValue(map[keys.at(i)]->isSvg()));
        obj.setProperty("isCssFile", QScriptValue(map[keys.at(i)]->isCssFile()));
        obj.setProperty("isJsFile", QScriptValue(map[keys.at(i)]->isJsFile()));
        obj.setProperty("isDocFile", QScriptValue(map[keys.at(i)]->isDocFile()));
        obj.setProperty("isAudioFile", QScriptValue(map[keys.at(i)]->isAudioFile()));
        obj.setProperty("isVideoFile", QScriptValue(map[keys.at(i)]->isVideoFile()));
        obj.setProperty("isAudioFile", QScriptValue(map[keys.at(i)]->isAudioFile()));
        obj.setProperty("isFontFile", QScriptValue(map[keys.at(i)]->isFontFile()));
        obj.setProperty("isOtherFile", QScriptValue(map[keys.at(i)]->isOtherFile()));
        obj.setProperty("isHttp200", QScriptValue(map[keys.at(i)]->isHttp200()));
        obj.setProperty("isHttp301", QScriptValue(map[keys.at(i)]->isHttp301()));
        obj.setProperty("isHttp302", QScriptValue(map[keys.at(i)]->isHttp302()));
        obj.setProperty("isHttp304", QScriptValue(map[keys.at(i)]->isHttp304()));
        obj.setProperty("isHttp404", QScriptValue(map[keys.at(i)]->isHttp404()));

        // 将当前对象push到数组内
        array.setProperty(i, obj);
    }
    return array;
}

QScriptValue ScriptBinding::wrapperSelector()
{
    // 暴露selector相关方法
    QScriptValue obj = engine->newObject();
    QScriptValue nativeMathod;

    nativeMathod = engine->newFunction(ScriptBinding::selectorClear);
    obj.setProperty("clear", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedIMG);
    obj.setProperty("img", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedPNG);
    obj.setProperty("png", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedGIF);
    obj.setProperty("gif", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedICO);
    obj.setProperty("ico", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedJPG);
    obj.setProperty("jpg", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedSVG);
    obj.setProperty("svg", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedDoc);
    obj.setProperty("doc", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedCSS);
    obj.setProperty("css", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedJS);
    obj.setProperty("js", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedTotalTimeout);
    obj.setProperty("totaltimeout", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedWaitTimeout);
    obj.setProperty("waittimeout", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedDownloadTimeout);
    obj.setProperty("downloadtimeout", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedDNSLookupTimeout);
    obj.setProperty("dnstimeout", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedNoneGZip);
    obj.setProperty("nonegzip", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedNoneCache);
    obj.setProperty("nonecache", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedNoneCDN);
    obj.setProperty("nonecdn", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedCookie);
    obj.setProperty("cookie", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedFromCDN);
    obj.setProperty("fromcdn", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedSizeOut);
    obj.setProperty("sizeout", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedHttp200);
    obj.setProperty("http200", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedHttp301);
    obj.setProperty("http301", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedHttp302);
    obj.setProperty("http302", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedHttp304);
    obj.setProperty("http304", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::selectedHttp404);
    obj.setProperty("http404", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::getSelectedData);
    obj.setProperty("get", nativeMathod);
    return obj;
}

// 所有暴露的内置函数均放置在此命名空间下
void ScriptBinding::initNativeMethodToRootSpace()
{
    QScriptValue nativeMathod;

    // selector 空间下所有函数统一包装
    getRootSpace().setProperty("selector", wrapperSelector());

    // 其他内置函数包装
    nativeMathod = engine->newFunction(ScriptBinding::getNetworkData);
    getRootSpace().setProperty("networkData", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::httpRequest);
    getRootSpace().setProperty("httpRequest", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::process);
    getRootSpace().setProperty("process", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::loadScript);
    getRootSpace().setProperty("loadScript", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::readFile);
    getRootSpace().setProperty("readFile", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::writeFile);
    getRootSpace().setProperty("writeFile", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::base64FromFile);
    getRootSpace().setProperty("base64FromFile", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::dataURIFromImage);
    getRootSpace().setProperty("dataURIFromImage", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::cpu);
    getRootSpace().setProperty("cpu", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::memory);
    getRootSpace().setProperty("memory", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::alert);
    getRootSpace().setProperty("alert", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::about);
    getRootSpace().setProperty("about", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::watchedFiles);
    getRootSpace().setProperty("watchedFiles", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::unWatcher);
    getRootSpace().setProperty("unWatcher", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::watchFile);
    getRootSpace().setProperty("watchFile", nativeMathod);

    nativeMathod = engine->newFunction(ScriptBinding::watcherClose);
    getRootSpace().setProperty("watcherClose", nativeMathod);
}

// 由js实现的扩展放置在此命名空间下
void ScriptBinding::initExpandMethodToRootSpace()
{
    QScriptValue obj = engine->newObject();
    getRootSpace().setProperty(EXPAND_METHOD, obj);
}


// console 相关内置方法实现
QScriptValue ScriptBinding::consoleLog(QScriptContext *context, QScriptEngine *interpreter)
{
    int argc = context->argumentCount();
    if (argc == 0) {
        return QScriptValue::UndefinedValue;
    }
    QStringList args;
    for (int i = 0; i < argc; ++i) {
        args.append(context->argument(i).toString());
    }
    QString output = args.join("\n");

    // 通过信号工厂单例发消息
    // 避免绑定js的方法是静态的无法调用实例方法的问题。
    ScriptSignalFactory* ssf = (new ScriptSignalFactory())->instantiat();
    ssf->fire(ssf->consoleLog, output);
    return QScriptValue::UndefinedValue;
}

QScriptValue ScriptBinding::consoleTime(QScriptContext *context, QScriptEngine *interpreter)
{
    int argc = context->argumentCount();
    if (argc == 0) {
        context->throwError("TypeError: Not enough arguments");
    }
    timeMap[context->argument(0).toString()] = QDateTime::currentDateTime().toMSecsSinceEpoch();
    return QScriptValue::UndefinedValue;
}

QScriptValue ScriptBinding::consoleTimeEnd(QScriptContext *context, QScriptEngine *interpreter)
{
    int argc = context->argumentCount();
    if (argc == 0) {
        context->throwError("TypeError: Not enough arguments");
    }

    QString arg = context->argument(0).toString();
    QString output = "0ms";

    if (timeMap.keys().contains(arg)) {
        QString timeout = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch() - timeMap[arg]);
        output = arg + ": " +  timeout + "ms";
        timeMap.remove(arg);
        ScriptSignalFactory* ssf = (new ScriptSignalFactory())->instantiat();
        ssf->fire(ssf->consoleLog, output);
        return QScriptValue(timeout.toDouble());
    } else {
        return QScriptValue::UndefinedValue;
    }
}

QScriptValue ScriptBinding::consoleDir(QScriptContext *context, QScriptEngine *interpreter)
{
    int argc = context->argumentCount();
    if (argc == 0) {
        return QScriptValue::UndefinedValue;
    }
    ScriptSignalFactory* ssf = (new ScriptSignalFactory())->instantiat();
    QScriptValue arg = context->argument(0);
    if (arg.isString()) {
      ssf->fire(ssf->consoleLog, "\"" + arg.toString() + "\"");
    } else if (arg.isBool() || arg.isNumber() ||
        arg.isUndefined() || arg.isNull() || arg.isRegExp()) {
        ssf->fire(ssf->consoleLog, arg.toString());
    } else if (arg.isFunction()) {
        ssf->fire(ssf->consoleLog, "[Function]");
    } else if (arg.isObject()) {
        // 无比蛋疼的方法开始了…… 吐……
        QStringList func;
        func << "(function(json) {"
            << "    var output = ['{'];"
            << "    var depth = 5;"
            << "    var cDepth = 0;"
            << "    function dir(obj) {"
            << "        if (cDepth >= depth) {"
            << "            output.push(', {...}');"
            << "            return;"
            << "        }"
            << "        for (var i in obj) {"
            << "            output.push(i + ': ');"
            << "            var o = obj[i] ;"
            << "            if (typeof o === 'string') {"
            << "                output.push('\"' + o + '\"');"
            << "            } else if (typeof o === 'number') {"
            << "                output.push(o);"
            << "          } else if (typeof o === 'undefined') {"
            << "                output.push('undefined');"
            << "            } else if (typeof o === 'function' && objectToString(o) != '[object RegExp]') {"
            << "                output.push('[Function]');"
            << "            } else if (typeof o === 'object' && objectToString(o) === '[object Null]') {"
            << "                output.push('null');"
            << "            } else if (typeof o === 'object' && objectToString(o) === '[object Date]') {"
            << "                output.push(o.toLocaleString());"
            << "            } else if (objectToString(o) === '[object RegExp]') {"
            << "                output.push(o.toString());"
            << "            } else if (typeof o === 'object' && objectToString(o) === '[object Object]') {"
            << "                output.push('{');"
            << "                dir(o);"
            << "                output.push('}');"
            << "            }"
            << "            output.push(', ');"
            << "        }"
            << "        output = output.slice(0, -1);"
            << "    };"
            << "    function objectToString(o) {"
            << "        return Object.prototype.toString.call(o);"
            << "    }"
            << "    dir(json);"
            << "    output.push('}');"
            << "    return output.join('');"
            << "});";
        QScriptValue dir = interpreter->evaluate(func.join("\n"));
        QScriptValue output = dir.call(interpreter->globalObject(), QScriptValueList()<<arg);
        ssf->fire(ssf->consoleLog, output.toString());
    }
    return QScriptValue::UndefinedValue;
}

/* 给JS暴露的network数据方法 */
QScriptValue ScriptBinding::getNetworkData(QScriptContext *context, QScriptEngine *interpreter)
{
    QMap<QString, MonitorData*> map = MonitorDataMap::getMonitorDataMap()->getData();
    return wrapperHttpDataArray(map, interpreter);
}

/* selecotor 控制相关方法实现 */

QScriptValue ScriptBinding::selectorClear(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->clear();
    return context->thisObject();
}

QScriptValue ScriptBinding::getSelectedData(QScriptContext *context, QScriptEngine *interpreter)
{
    /*
    App.selector.clear();
    App.selector.http200();
    var arr = App.selector.get();
    arr.length;
    */
    QMap<QString, MonitorData*> map = ScriptBinding::selector->get();
    QScriptValue data = wrapperHttpDataArray(map, interpreter);
    ScriptBinding::selector->clear();
    return data;
}

QScriptValue ScriptBinding::selectedIMG(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedIMG();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedPNG(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedPNG();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedGIF(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedGIF();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedICO(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedICO();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedJPG(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedJPG();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedSVG(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedSVG();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedDoc(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedDoc();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedCSS(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedCSS();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedJS(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedJS();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedNoneGZip(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedNoneGZip();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedNoneCache(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedNoneCache();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedNoneCDN(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedNoneCDN();
    return context->thisObject();
}


QScriptValue ScriptBinding::selectedCookie(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedCookie();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedHttp200(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedHttp200();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedHttp301(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedHttp301();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedHttp302(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedHttp302();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedHttp304(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedHttp304();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedHttp404(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedHttp404();
    return context->thisObject();
}


QScriptValue ScriptBinding::selectedFromCDN(QScriptContext *context, QScriptEngine *interpreter)
{
    ScriptBinding::selector->selectedFromCDN();
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedTotalTimeout(QScriptContext *context, QScriptEngine *interpreter)
{
    QScriptValue arg1 = context->argument(0);
    if (arg1.isNumber() && arg1.toInt32() > 0)
        ScriptBinding::selector->selectedTotalTimeout(arg1.toInt32());
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedWaitTimeout(QScriptContext *context, QScriptEngine *interpreter)
{
    QScriptValue arg1 = context->argument(0);
    if (arg1.isNumber() && arg1.toInt32() > 0)
        ScriptBinding::selector->selectedWaitTimeout(arg1.toInt32());
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedDownloadTimeout(QScriptContext *context, QScriptEngine *interpreter)
{
    QScriptValue arg1 = context->argument(0);
    if (arg1.isNumber() && arg1.toInt32() > 0)
        ScriptBinding::selector->selectedDownloadTimeout(arg1.toInt32());
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedDNSLookupTimeout(QScriptContext *context, QScriptEngine *interpreter)
{
    QScriptValue arg1 = context->argument(0);
    if (arg1.isNumber() && arg1.toInt32() > 0)
        ScriptBinding::selector->selectedDNSLookupTimeout(arg1.toInt32());
    return context->thisObject();
}

QScriptValue ScriptBinding::selectedSizeOut(QScriptContext *context, QScriptEngine *interpreter)
{
    QScriptValue arg1 = context->argument(0);
    if (arg1.isNumber() && arg1.toInt32() > 0)
        ScriptBinding::selector->selectedSizeOut(arg1.toInt32());
    return context->thisObject();
}


QScriptValue ScriptBinding::base64FromFile(QScriptContext *context, QScriptEngine *interpreter)
{
    QScriptValue path = context->argument(0);
    if (!path.isString())
        return QScriptValue(false);

    QFile file(path.toString());
    if (!file.open(QIODevice::ReadOnly)) {
        return QScriptValue(false);
    }
    return QScriptValue(QString::fromAscii(((file.readAll()).toBase64())));
}

QScriptValue ScriptBinding::dataURIFromImage(QScriptContext *context, QScriptEngine *interpreter)
{
    QScriptValue path = context->argument(0);

    if (!path.isString())
        return QScriptValue(false);

    QString imageFile = path.toString();
    QString fileType = imageFile.split(".").last().toLower();
    QStringList typeList;
    typeList << "jpg" << "jpeg" << "png" << "bmp" << "gif" << "ico";
    if (!typeList.contains(fileType)) {
        return QScriptValue(false);
    }

    QFile file(path.toString());
    if (!file.open(QIODevice::ReadOnly)) {
        return QScriptValue(false);
    }
    return QScriptValue("data:image/"+ fileType +";base64," +
                        QString::fromAscii(((file.readAll()).toBase64())));
}

QScriptValue ScriptBinding::watcherClose(QScriptContext *context, QScriptEngine *interpreter)
{
    fileSystemWatcher->watcherClose();
    return QScriptValue::UndefinedValue;
}

QScriptValue ScriptBinding::watchedFiles(QScriptContext *context, QScriptEngine *interpreter)
{
    QStringList watchedPathList = fileSystemWatcher->watchedPathList();
    int c = watchedPathList.size();
    QScriptValue arr = interpreter->newArray(c);
    for (int i = 0; i < c; ++i) {
        arr.setProperty(i, QScriptValue(watchedPathList.at(i)), QScriptValue::ReadOnly);
    }
    return arr;
}

QScriptValue ScriptBinding::watchFile(QScriptContext *context, QScriptEngine *interpreter)
{
    QScriptValue file = context->argument(0);
    QScriptValue func = context->argument(1);
    if(!file.isString() || !func.isFunction()) {
        return QScriptValue(false);
    }
    fileSystemWatcher->addWatchPath(file.toString());
    fileSystemWatcher->addWatchEvent(interpreter, func);
    return func;
}

QScriptValue ScriptBinding::unWatcher(QScriptContext *context, QScriptEngine *interpreter)
{
    QScriptValue arg = context->argument(0);

    if (arg.isString()) {
        fileSystemWatcher->removeWatchedPath(arg.toString());
        return QScriptValue::UndefinedValue;
    }
    if(arg.isFunction()) {
        fileSystemWatcher->removeWatchedEvent(arg);
    }
    return QScriptValue(false);

}

QScriptValue ScriptBinding::process(QScriptContext *context, QScriptEngine *interpreter)
{
    QString program;
    QStringList args;
    // 默认 30 秒执行超时
    int msecs = 30000;

    if (context->argumentCount() == 0) {
        return QScriptValue::UndefinedValue;
    }

    if (!context->argument(0).isString()) {
        return QScriptValue::UndefinedValue;
    }

    program = context->argument(0).toString().trimmed();
    if (program == "") {
        return QScriptValue::UndefinedValue;
    }

    if (!context->argument(1).isArray()) {
        args << "";
    }

    QScriptValue arr = context->argument(1);
    qint32 len = arr.property("length").toInt32();
    for (qint32 i = 0; i < len; ++i) {
        QScriptValue value = arr.property(i);
        if (value.isString() || value.isNumber()) {
            args << value.toString();
        } else {
            break;
        }
    }

    if (context->argument(2).isNumber()) {
        msecs = context->argument(2).toString().toInt();
    }

    QProcess process;
    process.start(program, args);
    QByteArray output;

    QTime time;
    time.start();
    int startTime = time.elapsed();

    // 每隔300ms输出一次，避免页面锁死
    while (!process.waitForFinished(300)) {
        // 如果开启debug的回调模式
        if (context->argument(3).isFunction()) {
            QString state;
            switch (process.state()) {
             case QProcess::NotRunning:
                state = "Error the process not running.";
                break;
             case QProcess::Starting:
                state = "The process starting.";
                break;
             case QProcess::Running:
                state = "QProcess::Running.";
                break;
            }

            context->argument(3)
                 .call(context->thisObject(),
                       QScriptValueList() <<
                       QScriptValue(state) <<
                       QScriptValue(QString(process.readAllStandardOutput()))
                       );
        }
        // 超时处理
        if (time.elapsed() - startTime > msecs) {
            if (!process.exitStatus() == QProcess::NormalExit) {
                process.kill();
            }
            return QScriptValue(QString(process.readAllStandardOutput()));
        }
        // 激活 UI 避免锁死
        qApp->processEvents();
    }
    // 300 ms 处理时间之内完成的内容 立即返回结果
    return QScriptValue(QString(process.readAllStandardOutput()));
}

QScriptValue ScriptBinding::httpRequest(QScriptContext *context, QScriptEngine *interpreter)
{
    enum HttpMethod {
        GET,
        POST
    };

    QString url = "";
    QString data ="";

    HttpMethod method = GET;

    if (context->argumentCount() == 0) {
        return QScriptValue::UndefinedValue;
    }

    if (context->argument(0).isString()) {
        QString arg = context->argument(0).toString().trimmed().toUpper();
        method = (arg == "POST") ?  POST : GET;
    }

    if (context->argument(1).isString()) {
        url = context->argument(1).toString().trimmed();
    }

    if (url.toLower().indexOf("http://") != 0) {
        context->throwError("URIError: URL is not http://");
    }

    if (context->argument(2).isString()) {
        data = context->argument(2).toString();
    }

    QNetworkReply* reply;
    QNetworkRequest req;
    QNetworkAccessManager* manager = new QNetworkAccessManager();

    if (method == POST) {
        // post 数据编码
        req.setUrl(QUrl(url));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        reply = manager->post(req, QUrl(data).toEncoded());
    } else {
        if (data != "") {
            if (url.indexOf("?") != -1) {
                if (url.lastIndexOf("&") == url.size() - 1) {
                    url = url + data;
                } else {
                    url = url + "&" + data;
                }
            } else {
                url= url + "?" + data;
            }
        }

        reply = manager->get(QNetworkRequest(QUrl(url)));
    }

    // 开启局部事件循环
    // 当finished信号触发时自动退出
    // 这样将异步信号变为同步
    QEventLoop eventLoop;
    connect(manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    QByteArray responseData;
    responseData = reply->readAll();

    QString charset = QString(reply->rawHeader("Content-Type")).toLower();
    QRegExp charsetRegExp("charset=([\\w-]+)\\b");
    int pos = charset.indexOf(charsetRegExp);
    if (pos > 0) {
        if (charsetRegExp.cap().size() < 2) {
            charset = "";
        } else {
            charset = charsetRegExp.cap(1);
        }

    } else {
        charset = "";
    }

    // 如用户设定编码则使用用户设定值
    if (context->argument(3).isString()) {
        charset = context->argument(3).toString().trimmed();
    }

    QTextStream stream(responseData);
    stream.setCodec(getCodec(charset));
    return QScriptValue(QString(stream.readAll()));
}


QScriptValue ScriptBinding::cpu(QScriptContext *context, QScriptEngine *interpreter)
{
    return QScriptValue(appInfo->getCPU());
}

QScriptValue ScriptBinding::memory(QScriptContext *context, QScriptEngine *interpreter)
{
    return QScriptValue(appInfo->getMemory());
}

QScriptValue ScriptBinding::alert(QScriptContext *context, QScriptEngine *interpreter)
{
    CommandParameters cmdParams;
    QString message;
    if (context->argumentCount() == 0) {
        message = "";
    } else {
        message = context->argument(0).toString();
    }

    QMessageBox messageBox;
    messageBox.information(NULL,
                           "App Message:",
                           message,
                           QMessageBox::Yes, QMessageBox::Yes);

    return QScriptValue::UndefinedValue;
}

QScriptValue ScriptBinding::about(QScriptContext *context, QScriptEngine *interpreter)
{
    QStringList about;
    about << "<h3 style=\"color:red;\">Welcome use of semi-finished products of the berserk</h3>"
          << "<p>Version: " << VERSION_STRING << "</p>"
          << ABOUT_STRING;
    QMessageBox messageBox;
    messageBox.about(NULL, "About", about.join(""));
    return QScriptValue::UndefinedValue;
}


