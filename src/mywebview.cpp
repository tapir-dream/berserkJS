#include "mywebview.h"
#include "pageextension.h"

#include <QTemporaryFile>
#include <QPrinter>

PageExtension* pageExtension;

MyWebView::MyWebView()
{        
    // 扩展页面对象
    pageExtension = new PageExtension(this);

    // 使用自扩展的 page 对象
    myPage = new MyWebPage();
    this->setPage(myPage);

    myFrame = myPage->mainFrame();

    // 初始化页面首次渲染标志
    firstPaintFinished = false;

    // 建立首屏监控对象
    firstScreen = new FirstScreen(myPage);

    // 设置新引用
    QNetworkAccessManager* oldManager = myPage->networkAccessManager();
    newManager = new NetworkAccessManager(oldManager, this);
    myPage->setNetworkAccessManager(newManager);

    // 设置cookie处理
    cookieJar = new QNetworkCookieJar();
    myPage->networkAccessManager()->setCookieJar(cookieJar);

    initEventNameMap();
    initEvents();

}

MyWebView::~MyWebView() {
   delete cookieJar;
   delete firstScreen;
   delete myPage;
   delete newManager;
}

void MyWebView::setAppScriptEngine(ScriptBinding* scriptBinding)
{
    initAppScriptEngine(scriptBinding);
    setAppScriptWebViewObject();
}

void MyWebView::initEventNameMap()
{
    // 构造js事件名与内置事件对应关系
    // 引用方式插入避免产生QScriptValueList的copy
    eventNameMap.insert("pagescriptcreated", &javaScriptWindowObjectClearedFunc);
    eventNameMap.insert("load", &loadFinishedFunc);
    eventNameMap.insert("initlayoutcompleted", &initialLayoutCompletedFunc);
    eventNameMap.insert("pagechanged", &pageChangedFunc);
    // This signal is emitted whenever the document content size change
    eventNameMap.insert("contentssizechanged", &contentsSizeChangedFunc);
    eventNameMap.insert("iconchanged", &iconChangedFunc);
    eventNameMap.insert("loadstarted", &loadStartedFunc);
    eventNameMap.insert("titlechanged", &titleChangedFunc);
    eventNameMap.insert("urlchanged",&urlChangedFunc);
    eventNameMap.insert("repaint", &repaintRequestedFunc);
    // This signal is emitted whenever the document wants to change the position and size of the page to geom.
    // example: window.moveBy/window.moveTo/window.resizeBy/window.resizeTo.
    eventNameMap.insert("pagerectchanged", &geometryChangeRequestedFunc);
    eventNameMap.insert("loadprogress", &loadProgressFunc);

    eventNameMap.insert("message", &pageMessageFunc);
    eventNameMap.insert("consolemessage", &pageConsoleMessageFunc);
    eventNameMap.insert("confirm", &pageConfirmFunc);
    eventNameMap.insert("alert", &pageAlertFunc);
    eventNameMap.insert("prompt", &pagePromptFunc);


    // 加入页面首次渲染完成事件回调
    eventNameMap.insert("firstpaintfinished", &pageFirstPaintFinishedFunc);
    // 加入页面首屏渲染完成事件回调
    eventNameMap.insert("firstscreenfinished", &pageFirstScreenFinishedFunc);

    // 加入每个请求开始时事件回调
    eventNameMap.insert("requeststart", &requestStartFunc);
    // 加入每个请求完成后事件回调
    eventNameMap.insert("requestfinished", &requestFinishedFunc);

}

void MyWebView::initEvents()
{
    // 委托页面所有连接在当前视图中打开
    myPage->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(myPage, SIGNAL(linkClicked(QUrl)), this, SLOT(onOpenUrl(QUrl)));

    // 对所有事件添加信号槽
    connect(myFrame, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));
    connect(myFrame, SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(onJavaScriptWindowObjectCleared()));
    connect(myFrame, SIGNAL(initialLayoutCompleted()), this, SLOT(onInitialLayoutCompleted()));
    connect(myFrame, SIGNAL(pageChanged()), this, SLOT(onPageChanged()));
    connect(myFrame, SIGNAL(contentsSizeChanged(const QSize)), this, SLOT(onContentsSizeChanged(const QSize)));
    connect(myFrame, SIGNAL(iconChanged()), this, SLOT(onIconChanged()));
    connect(myFrame, SIGNAL(loadStarted()), this, SLOT(onLoadStarted()));
    connect(myFrame, SIGNAL(titleChanged(const QString)), this, SLOT(onTitleChanged(const QString)));
    connect(myFrame, SIGNAL(urlChanged(const QUrl)), this, SLOT(onUrlChanged(const QUrl)));

    connect(myPage, SIGNAL(loadProgress(int)), this, SLOT(onLoadProgress(int)));
    connect(myPage, SIGNAL(repaintRequested(const QRect)), this, SLOT(onRepaintRequested(const QRect)));
    connect(myPage, SIGNAL(geometryChangeRequested(const QRect)), this, SLOT(onGeometryChangeRequested(const QRect)));

    connect(myPage, SIGNAL(pageConsoleMessage(QString,int,QString)), this, SLOT(onPageConsoleMessage(QString,int,QString)));
    connect(myPage, SIGNAL(pageAlert(QString)), this, SLOT(onPageAlert(QString)));
    connect(myPage, SIGNAL(pageConfirm(QString)), this, SLOT(onPageConfirm(QString)));
    connect(myPage, SIGNAL(pagePrompt(QString,QString)), this, SLOT(onPagePrompt(QString,QString)));




    connect(newManager, SIGNAL(requestFinished(QString)),
            this, SLOT(onRequestFinished(QString)));
    connect(newManager, SIGNAL(requestStart(QString)),
            this, SLOT(onRequestStart(QString)));
};

void MyWebView::initAppScriptEngine(ScriptBinding* scriptBinding)
{
    script = scriptBinding;
    appEngine = scriptBinding->getScriptEngine();
}

void MyWebView::setAppScriptWebViewObject()
{
    QScriptValue scriptWebView = appEngine->newQObject(this, QScriptEngine::QtOwnership);
    script->getRootSpace().setProperty("webview", scriptWebView);
}

QScriptValue MyWebView::getWebViewObjcet()
{
    return script->getRootSpace().property("webview");
}

QScriptValue MyWebView::cookiesFromUrl(QScriptValue url)
{
    QUrl targetUrl;
    if (url.isUndefined()) {
        targetUrl = this->url();
    } else if (url.toString().toLower().trimmed().indexOf("http://") == 0) {
        targetUrl = QUrl(url.toString().toLower().trimmed());
    } else {
        targetUrl = this->url();
    }

    QByteArray str;

    QNetworkCookie cookie;
    QList<QNetworkCookie> list = cookieJar->cookiesForUrl(targetUrl);
    foreach (cookie, list)
    {
        str += cookie.toRawForm() + "\n";
    }
    return QScriptValue(QString(str));
}

QScriptValue MyWebView::setCookiesFromUrl(QScriptValue cookie, QScriptValue url)
{
    QUrl targetUrl;
    QString targetCookie;

    if (!cookie.isString()) {
        return QScriptValue(false);
    }

    targetCookie = cookie.toString().trimmed();

    if (!url.isString()) {
        targetUrl = this->url();
    } else if (url.toString().toLower().trimmed().indexOf("http://") == 0) {
        targetUrl = QUrl(url.toString());
    }
    cookieJar->setCookiesFromUrl(QNetworkCookie::parseCookies(targetCookie.toUtf8()), targetUrl);
    return QScriptValue(true);
}

QScriptValue MyWebView::open(QScriptValue url)
{
    if (!url.isString()) {
        return QScriptValue(false);
    }

    QString cUrl = url.toString();

    if (cUrl.toLower().indexOf("http") == 0 ||
        cUrl.toLower().indexOf("https") == 0 ||
        cUrl.toLower().indexOf("ftp") == 0 ||
        cUrl.toLower().indexOf("file") == 0) {

        if (QUrl(cUrl).isEmpty()) {
            return QScriptValue(false);
        }

        /* 强制请求从缓存出
        QNetworkRequest request;
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
        request.setUrl(QUrl(cUrl));
        this->load(request);
        */

        this->load(QUrl(cUrl));

        return QScriptValue(true);
    } else {
       return QScriptValue(false);
    }
}

QScriptValue MyWebView::getUrl()
{
    return QScriptValue(myFrame->url().toString());
}

QScriptValue MyWebView::netListener(QScriptValue enabled)
{
    if (enabled.toBool() == true) {
        NetworkAccessManager::isListener = true;
        MonitorDataMap::getMonitorDataMap()->clear();
    } else {
        NetworkAccessManager::isListener = false;
    }
    return QScriptValue(true);
}

QString MyWebView::jsonStringify(QScriptValue scriptObject)
{
    QScriptValue json = appEngine->globalObject().property("JSON");
    json = json.property("stringify")
            .call(json, QScriptValueList() << scriptObject);
    QString argsToJsonStr = json.toString();
    int stringLength = argsToJsonStr.size();
    // 如果是字符串，首尾会有"字符，否则为非字符串的转义处理
    /*
    .replace("\\n", "\\\\n")
    .replace("\\r'", "\\\\r")
    .replace("\\t'", "\\\\t")
    .replace("\b'", "\\\\b")
    .replace("\\\"", "\\\\\"")
    .replace("\\\'", "\\\\\'")
    .replace("\"", "\\\"")
    .replace("\'","\\\'");
    */
    //qDebug()<<argsToJsonStr;
    if (argsToJsonStr.mid(0, 1) == "\"" && argsToJsonStr.mid(stringLength - 1, stringLength) == "\"") {
        argsToJsonStr = argsToJsonStr.mid(1, stringLength - 2);
        argsToJsonStr.replace("\\", "\\\\");
        argsToJsonStr.replace("\\\\\"", "\\\\\\\"");
        argsToJsonStr = "\\\"" + argsToJsonStr + "\\\"";
    } else {
        argsToJsonStr.replace("\\", "\\\\");
        argsToJsonStr.replace("\"", "\\\"");
    }
    return argsToJsonStr;
}

QScriptValue MyWebView::jsonParse(QScriptValue jsonString)
{
    if (!jsonString.isString())
        return QScriptValue::UndefinedValue;
    QScriptValue json = appEngine->globalObject().property("JSON");
    return json.property("parse").call(json, QScriptValueList() << jsonString);
}

QScriptValue MyWebView::execScript(QScriptValue scriptFunc, QScriptValue args)
{
    if (!scriptFunc.isFunction())
        return QScriptValue::UndefinedValue;

    QVariant result;

    if (args.isUndefined()) {

        result = myFrame->evaluateJavaScript(
            "JSON.stringify(("+ scriptFunc.toString() +")())");
        if (result.isNull())
            return QScriptValue::UndefinedValue;
        return jsonParse(result.toString());
    }

    /* 使用JSON.parse与JSON.stringify将对象序列化与反序列化，便于跨脚本沙箱传递js的object。
    (function (o) {
      return JSON.parse(JSON.stringify(
        (function(o){
           return (function (o){
                      return o;
                  })(JSON.parse(o))
        })(JSON.stringify(o))
      ))
    })({x: 20});
    */

    QString argsToJsonStr = jsonStringify(args);

    result = myFrame->evaluateJavaScript("JSON.stringify((function(param){ return (" +
                                         scriptFunc.toString() +
                                         ")(JSON.parse(param))" +
                                         "})(\"" + argsToJsonStr + "\"))");
    if (result.isNull())
         return QScriptValue::UndefinedValue;

    return jsonParse(result.toString());
}

QScriptValue MyWebView::viewport()
{
    return sizeToScriptObject(myPage->viewportSize());
}

QScriptValue MyWebView::setViewport(QScriptValue size)
{
    if (!size.isObject())
        return QScriptValue(false);
    if (!size.property("width").isNumber() || !size.property("height").isNumber())
        return QScriptValue(false);
    QSize viewportSize(size.property("width").toString().toInt(),
                       size.property("height").toString().toInt());
    myPage->setViewportSize(viewportSize);
    return QScriptValue(true);
}

QScriptValue MyWebView::timer(QScriptValue scriptFunc, QScriptValue timeout, bool singleShot)
{
    if (scriptFunc.isString()) {
        scriptFunc = QScriptValue("function () {" + scriptFunc.toString() + "}");
    }
    if (!scriptFunc.isFunction()) {
        return QScriptValue::UndefinedValue;
    }
    if (timeout.toInt32() < 0) {
        timeout = QScriptValue(0);
    }
    QTimer* timer = new QTimer(this);

    if (singleShot) {
        timer->setSingleShot(true);
    }

    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(timeout.toString().toInt());

    // start 后才有 id 可用。
    QString key = QString::number(timer->timerId());
    timer->setObjectName(key);

    // 记录相关环境内容
    ContextInfo contextInfo;
    contextInfo.thisObject = appEngine->currentContext()->thisObject();
    contextInfo.activationObject = appEngine->currentContext()->activationObject();
    contextInfo.func = scriptFunc;

    TimerInfo timerInfo;
    timerInfo.info = contextInfo;
    timerInfo.timer = timer;
    timeEventMap.insert(key, timerInfo);

    return QScriptValue(key.toInt());
}

QScriptValue MyWebView::clearTimer(QScriptValue timeId)
{
    if (!timeId.isNumber()) {
        return QScriptValue(false);
    }
    killTimer(timeId.toInt32());
    return QScriptValue(true);
}

QScriptValue MyWebView::setTimeout(QScriptValue scriptFunc, QScriptValue timeout)
{
    return timer(scriptFunc, timeout, true);
}

QScriptValue MyWebView::setInterval(QScriptValue scriptFunc, QScriptValue timeout)
{
    return timer(scriptFunc, timeout, false);
}

QScriptValue MyWebView::clearTimeout(QScriptValue timeId)
{
    if (!timeId.isNumber() || timeId.toInt32() == -1)
        return QScriptValue(false);
    return clearTimer(timeId);
}

QScriptValue MyWebView::clearInterval(QScriptValue timeId)
{

    if (!timeId.isNumber() || timeId.toInt32() == -1) {
        return QScriptValue(false);
    }
    QString key = timeId.toString();

    if (!timeEventMap.contains(key))
        return QScriptValue(false);

    delete timeEventMap[key].timer;
    timeEventMap.remove(key);
    return clearTimer(timeId);
}

QScriptValue MyWebView::setProxy(QScriptValue host, QScriptValue type,
                                 QScriptValue userName, QScriptValue password)
{
    if (!host.isString()) {
        return QScriptValue(false);
    }

    QNetworkProxy proxy;

    QStringList cHost = host.toString().split(":");
    proxy.setHostName(cHost.at(0));

    if (cHost.size() > 1) {
       proxy.setPort(cHost.at(1).toInt());
    }

    QMap<QString, QNetworkProxy::ProxyType> cType;
    cType.insert("HTTP", QNetworkProxy::HttpProxy);
    cType.insert("SOCKS5", QNetworkProxy::Socks5Proxy);

    if (!type.isString()) {
       proxy.setType(QNetworkProxy::HttpProxy);
    } else if (cType.contains(type.toString().toUpper())) {
        proxy.setType(cType[type.toString().toUpper()]);
    } else {
        proxy.setType(QNetworkProxy::HttpProxy);
    }

    if (userName.isString()) {
        proxy.setUser(userName.toString());

    }

    if (password.isString()) {
        proxy.setPassword(password.toString());
    }
    myPage->networkAccessManager()->setProxy(proxy);
    return QScriptValue(true);
}

QScriptValue MyWebView::clearProxy()
{
    myPage->networkAccessManager()->setProxy(QNetworkProxy::NoProxy);
    return QScriptValue(true);
}

QScriptValue MyWebView::useSystemProxy(QScriptValue index)
{
    int num;

    if (!(index.isString() || index.isNumber())) {
        return QScriptValue(false);
    }

    num = index.toString().toInt();
   ;

    QList<QNetworkProxy> proxyList =
            QNetworkProxyFactory::systemProxyForQuery(
                QNetworkProxyQuery(QUrl("http://localhost/")));

    if (proxyList.size() == 0) {
        return QScriptValue(false);
    }

    if (proxyList.size() < num) {
        num = proxyList.size() - 1;
    }
    if (proxyList.at(num).hostName().trimmed().isEmpty()) {
         return QScriptValue(false);
    }

    myPage->networkAccessManager()->setProxy(proxyList.at(num));
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    return QScriptValue(true);
}

QScriptValue MyWebView::setUserAgent(QScriptValue userAgent)
{
    if (!userAgent.isString() ||
         userAgent.toString().trimmed().isEmpty()) {
        return QScriptValue(false);
    }
    return QScriptValue(myPage->setUserAgent(userAgent.toString().trimmed()));
}

QScriptValue MyWebView::userAgent()
{
    return QScriptValue(myPage->userAgent());
}

QScriptValue MyWebView::defaultUserAgent()
{
    return QScriptValue(myPage->defaultUserAgent);
}

QScriptValue MyWebView::contentRect()
{
    QScriptValue rect = sizeToScriptObject(myFrame->contentsSize());
    rect.setProperty("x", QScriptValue(0));
    rect.setProperty("y", QScriptValue(0));
    return rect;
}

QImage MyWebView::renderToImage()
{
    // page content size
    QSize size = myFrame->contentsSize();
    QSize oldSize = myPage->viewportSize();
    myPage->setViewportSize(myFrame->contentsSize());

    QImage image(size, QImage::Format_ARGB32);
    // transprent background
    image.fill(Qt::transparent);

    // render the web page in QImage Object.
    QPainter p;
    p.begin(&image);

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    myFrame->render(&p);
    p.end();
    // reset old option
    myPage->setViewportSize(oldSize);
    return image;
}

void MyWebView::fixClipRectToRenderRect(QRect* clipRect, QImage* image )
{
    // 此处做特殊处理
    // 如果给定的截图区域超过图片宽高则将多余部分省去
    // 避免出现白边和黑边
    int imgW = image->width();
    int imgH = image->height();
    int clipW = clipRect->x() + clipRect->width();
    int clipH = clipRect->y() + clipRect->height();

    if (clipRect->x() > imgW) {
        clipRect->setX(imgW);
    }
    if (clipRect->y() > imgH) {
        clipRect->setY(imgH);
    }

    if (clipW > imgW) {
        clipRect->setWidth(imgW - clipRect->x());
    }
    if (clipH > imgH) {
        clipRect->setHeight(imgH - clipRect->y());
    }
}

QScriptValue MyWebView::dataURIFromRect(QScriptValue rect, QScriptValue type, QScriptValue quality)
{
    // 参数初始化
    int q = quality.toString().toInt();
    q = (q == 0) ? -1 : q;
    q = (q > 100) ? 100 : q;

    QString fileType = type.toString().toUpper();
    QStringList typeList;
    typeList << "PNG" << "JPG" << "JPEG" << "BMP";
    if (!typeList.contains(fileType)) {
        fileType = typeList.at(0);
    }

    if (!rect.isObject() ||
        !rect.property("x").isNumber() ||
        !rect.property("y").isNumber() ||
        !rect.property("width").isNumber() ||
        !rect.property("height").isNumber()) {
        rect = contentRect();
    }

    QRect clipRect;
    clipRect.setX(rect.property("x").toString().toInt());
    clipRect.setY(rect.property("y").toString().toInt());
    clipRect.setWidth(rect.property("width").toString().toInt());
    clipRect.setHeight(rect.property("height").toString().toInt());
    QImage image = renderToImage();
    fixClipRectToRenderRect(&clipRect, &image);

    // 把修正后的区域截取出来
    image = image.copy(clipRect);

    // 创建临时文件
    QTemporaryFile tmpFile;
    if (!tmpFile.open()) {
        return QScriptValue(false);
    }

    image.save(tmpFile.fileName(), fileType.toAscii(), q);
    QScriptValue dataURI = QScriptValue("data:image/"+ fileType.toLower() +";base64," +
                            QString::fromAscii(((tmpFile.readAll()).toBase64())));
    tmpFile.close();
    return dataURI;
}

bool MyWebView::clipRenderToImage(QString path, QString type, int quality, QRect clipRect)
{
    QImage image = renderToImage();
    type = type.toUpper();
    QStringList typeList;
    typeList << "PNG" << "JPG" << "JPEG" << "BMP" << "PPM" << "TIFF";
    if (!typeList.contains(type)) {
        type = typeList.at(0);
    }
    quality = (quality > 100) ? 100 : quality;
    quality = (quality < 0) ? -1 : quality;

    fixClipRectToRenderRect(&clipRect, &image);

    if (!clipRect.isEmpty()) {
        image = image.copy(clipRect);
    }

    bool isSaved = image.save(path, type.toAscii(), quality);

    return isSaved;
}

QScriptValue MyWebView::saveImage(QScriptValue path, QScriptValue type, QScriptValue quality, QScriptValue rect)
{
    if (!path.isString())
        return QScriptValue(false);
    int q = quality.toString().toInt();
    q = (q == 0) ? -1 : q;
    q = (q > 100) ? 100 : q;

    if (rect.isObject() &&
        rect.property("x").isNumber() &&
        rect.property("y").isNumber() &&
        rect.property("width").isNumber() &&
        rect.property("height").isNumber()) {

        QRect clipRect;
        clipRect.setX(rect.property("x").toString().toInt());
        clipRect.setY(rect.property("y").toString().toInt());
        clipRect.setWidth(rect.property("width").toString().toInt());
        clipRect.setHeight(rect.property("height").toString().toInt());

        return  QScriptValue(clipRenderToImage(path.toString(), type.toString(), q, clipRect));
    }

    return QScriptValue(clipRenderToImage(path.toString(), type.toString(), q));
}

QScriptValue MyWebView::savePdf(QScriptValue path)
{
    // page content size
    QSize oldSize = myPage->viewportSize();
    QSize contentsSize = myFrame->contentsSize();
    myPage->setViewportSize(contentsSize);
    QSizeF size(contentsSize);
    // render the web page in Printer Object.
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path.toString());
    //printer.setResolution(72);
    printer.setPaperSize(size, QPrinter::DevicePixel);
    printer.setFullPage(true);
    QPainter p;
    p.begin(&printer);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    myFrame->render(&p);
    p.end();
    // reset old option.
    myPage->setViewportSize(oldSize);
    return true;
}

QScriptValue MyWebView::elementRects(QScriptValue cssSelector)
{
    if (!cssSelector.isString())
        return QScriptValue::UndefinedValue;
    QScriptValue arr = appEngine->newArray();
    QWebElementCollection elements = myFrame->findAllElements(cssSelector.toString());
    int c = elements.count();
    if (c == 0)
        return QScriptValue::UndefinedValue;

    for (int i = 0; i < c; ++i) {
        arr.setProperty(i, rectToScriptObject(elements.at(i).geometry()));
    }
    return arr;
}

QScriptValue MyWebView::rectToScriptObject(const QRect & rect)
{
    QScriptValue object = appEngine->newObject();
    object.setProperty("x", rect.x(), QScriptValue::ReadOnly);
    object.setProperty("y", rect.y(), QScriptValue::ReadOnly);
    object.setProperty("width", rect.width(), QScriptValue::ReadOnly);
    object.setProperty("height", rect.height(), QScriptValue::ReadOnly);
    object.setProperty("left", rect.left(), QScriptValue::ReadOnly);
    object.setProperty("right", rect.right(), QScriptValue::ReadOnly);
    object.setProperty("top", rect.top(), QScriptValue::ReadOnly);
    object.setProperty("bottom", rect.bottom(), QScriptValue::ReadOnly);
    return object;
}

QScriptValue MyWebView::sizeToScriptObject(const QSize & size)
{
    QScriptValue object = appEngine->newObject();
    object.setProperty("width", size.width(), QScriptValue::ReadOnly);
    object.setProperty("height", size.height(), QScriptValue::ReadOnly);
    return object;
}


QScriptValue MyWebView::hasDetectionRects()
{
    return customFirstScreenDetectionPoints.size() > 0;
}

QScriptValue MyWebView::clearDetectionRects()
{
    customFirstScreenDetectionPoints.clear();
    return QScriptValue(true);
}

QScriptValue MyWebView::setDetectionRects(QScriptValue rects, QScriptValue sameRate)
{
    if (!rects.isArray()) {
        return QScriptValue(false);
    }

    if (!rects.property("length").isNumber()) {
        return QScriptValue(false);
    }

    qint32 len =rects.property("length").toInt32();
    if (len <= 0) {
        return QScriptValue(false);
    }

    // 准确率设置错误校验
    if (!sameRate.isNumber()) {
        customFirstScreenDetectionPointsSameRate = 0.95;
    } else {
        customFirstScreenDetectionPointsSameRate =
                sameRate.toString().toFloat();
    }

    // 准确率校准在 0-1 之间
    if (customFirstScreenDetectionPointsSameRate > 1.0) {
        customFirstScreenDetectionPointsSameRate = 1.0;
    }

    if (customFirstScreenDetectionPointsSameRate < 0.0) {
        customFirstScreenDetectionPointsSameRate = 0.0;
    }

    QSize viewport = myPage->viewportSize();

    for (int i = 0; i < len; ++i) {
        QScriptValue rectObject = rects.property(i);
        // 排除不符合rect对象属性要求的数据
        if (!rectObject.isObject() ||
            !rectObject.property("x").isNumber() ||
            !rectObject.property("y").isNumber() ||
            !rectObject.property("width").isNumber() ||
            !rectObject.property("height").isNumber()) {
            continue;
        }
        // 符合属性的建立为Rect内置对象
        QRect rect;
        rect.setX(rectObject.property("x").toString().toInt());
        rect.setY(rectObject.property("y").toString().toInt());
        rect.setWidth(rectObject.property("width").toString().toInt());
        rect.setHeight(rectObject.property("height").toString().toInt());
        fixRectToViewport(&rect, &viewport);
        rectToPoints(&rect, &customFirstScreenDetectionPoints);
    }
    return QScriptValue(true);
}

void MyWebView::fixRectToViewport(QRect* rect, QSize* viewport)
{
    int maxWidth = viewport->width();
    int maxHeight = viewport->height();

    int x = rect->x();
    int y = rect->y();
    int width = rect->width();
    int height = rect->height();

    // 定位超出视口宽
    if (x > maxWidth) {
        rect->setX(maxWidth);
        rect->setWidth(0);
    }

    // 定位超出视口高
    if (y > maxHeight) {
        rect->setY(maxHeight);
        rect->setHeight(0);
    }

    // 宽度超出视口宽
    if (x + width > maxWidth) {
        rect->setWidth(maxWidth - x);
    }

    // 高度超出视口高
    if (y + height > maxHeight) {
        rect->setWidth(maxHeight - y);
    }

    // 负向超出视口
    if (x < 0) {
        rect->setX(0);
        rect->setWidth((x + width) < 0 ? 0 : (x + width));
    }

    if (y < 0) {
        rect->setY(0);
        rect->setHeight((y + height) < 0 ? 0 : (y + height));
    }
}

void MyWebView::rectToPoints(QRect* rect, QList<QPoint>* points) {

    int width = rect->width();
    int height = rect->height();

    int x = rect->x();
    int y = rect->y();

    int maxWidth = x + width - 1;
    int maxHeight = y + height - 1;

    for (; x <= maxWidth; ++x) {
        for (; y <= maxHeight; ++y) {
            points->append(QPoint(x, y));
        }
    }
}

QScriptValue MyWebView::addEventListener(QScriptValue eventName, QScriptValue scriptFunc)
{
    if (!eventName.isString())
        return QScriptValue::UndefinedValue;
    if (!scriptFunc.isFunction())
        return QScriptValue::UndefinedValue;

    QString event = eventName.toString().toLower();

    if (!eventNameMap.contains(event))
        return QScriptValue::UndefinedValue;


    // 相同回调仅注册一次
    QList<ContextInfo>* callbacks = eventNameMap[event];
    int c = callbacks->size();
    for(int i = 0; i < c; ++i) {
        if (callbacks->at(i).func.toString() == scriptFunc.toString()) {
            return scriptFunc;
        }
    }

    // 保留当前运行时所需环境: activationObject/thisObject
    ContextInfo contextInfo;
    contextInfo.activationObject = appEngine->currentContext()->activationObject();
    contextInfo.thisObject = appEngine->currentContext()->thisObject();
    contextInfo.func = scriptFunc;

    eventNameMap[event]->append(contextInfo);
    return scriptFunc;
}

QScriptValue MyWebView::removeEventListener(QScriptValue eventName, QScriptValue scriptFunc)
{
    bool isRemoved = false;
    if (!eventName.isString())
        return QScriptValue::UndefinedValue;
    if (!scriptFunc.isFunction())
        return QScriptValue::UndefinedValue;

    QString event = eventName.toString().toLower();

    if (!eventNameMap.contains(event))
        return QScriptValue::UndefinedValue;

    QString funcStr = scriptFunc.toString();

    QList<ContextInfo>* callbacks = eventNameMap[event];
    int c = callbacks->size();
    for(int i = 0; i < c; ++i) {
        if (callbacks->at(i).func.toString() == funcStr) {
            callbacks->removeAt(i);
            isRemoved = true;
            break;
        }
    }
    return QScriptValue(isRemoved);
}

void MyWebView::normalFireEvent(QList<ContextInfo> eventHandleList)
{
    int c = eventHandleList.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = eventHandleList.at(i);
            // 将函数所在的scope设置为函数所在激活对象的sope
            // 即将函数执行放于它定义时的scope内
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            // 执行函数
            contextInfo.func.call(contextInfo.thisObject);
        }
    }
}

void MyWebView::onRepaintRequested(const QRect & dirtyRect)
{
    // 第一次 repaint 时，将首次渲染标志设置为true。
    if (!firstPaintFinished) {
        firstPaintFinished = true;
        qint64 firePaintTimeout =
                QDateTime::currentDateTime().toMSecsSinceEpoch()
                - urlChangedTime;
        // 负值修正
        if (firePaintTimeout < 0)
             firePaintTimeout = 0;

        int c = pageFirstPaintFinishedFunc.size();
        if (c > 0) {
            for (int i = 0; i < c; ++i) {
                ContextInfo contextInfo = pageFirstPaintFinishedFunc.at(i);
                contextInfo.func.setScope(contextInfo.activationObject.scope());
                contextInfo
                    .func.call(
                      contextInfo.thisObject,
                      QScriptValueList()
                        << QScriptValue(QString::number(firePaintTimeout).toInt())
                        << QScriptValue(myFrame->url().toString())
                      );
            }
        }
        firstScreen->setStartScanViewTime(firePaintTimeout);
    }

    int c = repaintRequestedFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = repaintRequestedFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject, QScriptValueList() << rectToScriptObject(dirtyRect));
        }
    }
}

void MyWebView::onGeometryChangeRequested(const QRect & geom)
{
    int c = geometryChangeRequestedFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = geometryChangeRequestedFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject, QScriptValueList() << rectToScriptObject(geom));
        }
    }
}

void MyWebView::onContentsSizeChanged (const QSize & size)
{
    int c = contentsSizeChangedFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = contentsSizeChangedFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject, QScriptValueList() << sizeToScriptObject(size));
        }
    }
}

void MyWebView::onLoadProgress(int progress)
{
    int c = loadProgressFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = loadProgressFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject, QScriptValueList() << QScriptValue(progress));
        }
    }
}

void MyWebView::onTitleChanged(const QString & title)
{
    int c = titleChangedFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = titleChangedFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject, QScriptValueList() << QScriptValue(title));
        }
    }
}

void MyWebView::onUrlChanged(const QUrl & url)
{
    // 如果URL变更了并且监听了任意回调（首屏回调依赖于首次渲染）
    // 那么页面的首次渲染标记将被设置为flase
    // 这个标志等到第一次repaint事件被触发时才被设置为true
    if (firstPaintFinished &&
            (pageFirstPaintFinishedFunc.size() > 0 ||
             pageFirstScreenFinishedFunc.size() >0)) {
        firstPaintFinished = false;
        // 设置页面变更时间戳
        urlChangedTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    // 如果 URL 变更了
    // 存在事件回调注册（优化无事件绑定时性能）
    // 则新建首屏对象
    if (pageFirstScreenFinishedFunc.size() > 0) {
        disconnect(firstScreen, SIGNAL(firstScreenRenderFinish(int, QString)),
                   this, SLOT(onFirstScreenRenderTimeout(int, QString)));
        firstScreen->close();
        delete firstScreen;

        firstScreen = new FirstScreen(myPage);
        // 如果脚本中设置了自定义监测点集合，那么就采用自定义点集来计算首屏时间。
        // 否则默认采用均匀分布点集监测方式。
        if (customFirstScreenDetectionPoints.size() != 0) {
            firstScreen->setCustomDetectionPoints(
                        customFirstScreenDetectionPoints,
                        customFirstScreenDetectionPointsSameRate);
        }
        connect(firstScreen, SIGNAL(firstScreenRenderFinish(int, QString)),
                   this, SLOT(onFirstScreenRenderTimeout(int, QString)));
    }

    // 执行事件监听回调
    int c = urlChangedFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = urlChangedFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject, QScriptValueList() << QScriptValue(url.toString()));
        }
    }
}

void MyWebView::onFirstScreenRenderTimeout(int timeout, QString url)
{
    int c = pageFirstScreenFinishedFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = pageFirstScreenFinishedFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject,
                                  QScriptValueList()
                                  << QScriptValue(timeout)
                                  << QScriptValue(url));
        }
    }
}

void MyWebView::onIconChanged()
{
    normalFireEvent(iconChangedFunc);
}

void MyWebView::onLoadStarted()
{
    normalFireEvent(loadStartedFunc);
}

void MyWebView::onJavaScriptWindowObjectCleared()
{
    myFrame->addToJavaScriptWindowObject("__pageExtension", pageExtension);
    QStringList postMessageScript;
    postMessageScript
            << "__pageExtension.postMessage = function(wparam, lparam) {"
            << "wparam = (wparam === void 0) ? '' : wparam;"
            << "lparam = (lparam === void 0) ? '' : lparam;"
            << "__pageExtension.message(JSON.stringify(wparam), JSON.stringify(lparam));"
            << "}";
    myFrame->evaluateJavaScript(postMessageScript.join(""));
    normalFireEvent(javaScriptWindowObjectClearedFunc);
}

void MyWebView::onInitialLayoutCompleted()
{
    normalFireEvent(initialLayoutCompletedFunc);
}

void MyWebView::onPageChanged()
{
    normalFireEvent(pageChangedFunc);
}

void MyWebView::onOpenUrl(QUrl targetURL)
{
    this->setUrl(targetURL.toString());
}

void MyWebView::sendPageMessage(QString wparam, QString lparam)
{
    onPageMessage(wparam, lparam);
}

void MyWebView::onLoadFinished(bool ok)
{
    int c = loadFinishedFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = loadFinishedFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject,
                                  QScriptValueList()
                                  << QScriptValue(this->url().toString()));
        }
    }
}


void MyWebView::onPageConsoleMessage(QString message, int lineNumber, QString sourceID)
{
    int c = pageConsoleMessageFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = pageConsoleMessageFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject,
                                  QScriptValueList()
                                  << QScriptValue(message)
                                  << QScriptValue(lineNumber)
                                  << QScriptValue(sourceID));
        }
    }
}

void MyWebView::onPagePrompt(QString msg, QString defaultValue)
{
    int c = pagePromptFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = pagePromptFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject,
                                  QScriptValueList()
                                  << QScriptValue(msg)
                                  << QScriptValue(defaultValue));
        }
    }
}

void MyWebView::onPageConfirm(QString msg)
{
    int c = pageConfirmFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = pageConfirmFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject,
                                  QScriptValueList()
                                  << QScriptValue(msg));
        }
    }
}

void MyWebView::onPageAlert(QString msg)
{
    int c = pageAlertFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = pageAlertFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject,
                                  QScriptValueList()
                                  << QScriptValue(msg));
        }
    }
}

void MyWebView::onPageMessage(QString wparam, QString lparam)
{
    int c = pageMessageFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = pageMessageFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject,
                                  QScriptValueList()
                                  << jsonParse(wparam)
                                  << jsonParse(lparam));
        }
    }
}

void MyWebView::onRequestStart(QString url)
{
    int c = requestStartFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = requestStartFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject,
                                  QScriptValueList()
                                  << QScriptValue(url));
        }
    }
}

void MyWebView::onRequestFinished(QString url)
{
    int c = requestFinishedFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = requestFinishedFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject,
                                  QScriptValueList()
                                  << QScriptValue(url));
        }
    }
}

void MyWebView::onTimeout()
{
    QTimer* timer = qobject_cast<QTimer *>(sender());
    QString key = timer->objectName();

    if (!timeEventMap.contains(key))
        return;

    ContextInfo contextInfo = timeEventMap[key].info;
    contextInfo.func.setScope(contextInfo.activationObject);
    contextInfo.func.call(contextInfo.thisObject);
    if (timer->isSingleShot()) {
        timeEventMap.remove(key);
        delete timer;
    }
}

QString MyWebView::sizeToJson(const QSize & size)
{
    return "{width: "+ QString::number(size.width()) +
            ", height: "+ QString::number(size.height()) + "}";
}

QString MyWebView::rectToJson(const QRect & rect)
{
    return "{x: "+ QString::number(rect.x()) +
            ", y: "+ QString::number(rect.y()) +
            ", left: "+ QString::number(rect.left()) +
            ", right: "+ QString::number(rect.right()) +
            ", top: "+ QString::number(rect.top()) +
            ", bottom: "+ QString::number(rect.bottom()) +
            ", width: "+ QString::number(rect.width()) +
            ", height: "+ QString::number(rect.height()) +
            "}";
}
