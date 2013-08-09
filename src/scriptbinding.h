#ifndef SCRIPTBINDING_H
#define SCRIPTBINDING_H

#include <QObject>
#include <QtScript>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMessageBox>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include "monitordatamap.h"
#include "selector.h"
#include "filesystemwatcher.h"
#include "appinfo.h"


/**
 * ScriptBinding 类
 * 用来暴露内置属性与方法给JS调用。
 *
 * @author Tapir
 * @date   2012-02-17
 */


/**
 * Namespace diagram
 *
 * Global + [object] APP + [function] networkData
 * print  |              + [function] close
 * gc     |              + [function] loadScript
 * version|              + [function] readFile
 *        |              + [function] writeFile
 *        |              + [function] base64FrommFile
 *        |              + [function] dataURIFromImage
 *        |              + [function] netListener <--referenced from webview.netListener method
 *        |              + [function] process
 *        |              + [function] httpRequest
 *        |              + [function] watchFile
 *        |              + [function] unWatcher
 *        |              + [function] watchedFiles
 *        |              + [function] watcherClose
 *        |              + [function] cpu
 *        |              + [function] memory
 *        |              + [function] hide
 *        |              + [function] show
 *        |              + [function] alert
 *        |              + [function] about
 *        |              + [function] isOnline
 *        |              + [string] path
 *        |              + [string] file
 *        |              + [array] args
 *        |              + [object] selector + [function] clear
 *        |              |                   + [function] img
 *        |              |                   + [function] png
 *        |              |                   + [function] gif
 *        |              |                   + [function] ico
 *        |              |                   + [function] jpg
 *        |              |                   + [function] svg
 *        |              |                   + [function] doc
 *        |              |                   + [function] css
 *        |              |                   + [function] js
 *        |              |                   + [function] cookie;
 *        |              |                   + [function] nonegzip;
 *        |              |                   + [function] nonecache;
 *        |              |                   + [function] nonecdn;
 *        |              |                   + [function] totaltimeout
 *        |              |                   + [function] waittimeout
 *        |              |                   + [function] downloadtimeout
 *        |              |                   + [function] dnstimeout
 *        |              |                   + [function] sizeout
 *        |              |                   + [function] http200
 *        |              |                   + [function] http301
 *        |              |                   + [function] http302
 *        |              |                   + [function] http304
 *        |              |                   + [function] http404
 *        |              |                   + [function] fromcdn
 *        |              |
 *        |              + [object] webview + [function] getUrl
 *        |                              + [function] setUrl
 *        |                              + [function] execScript <-- *Page script engine environment
 *        |                              + [function] setTimeout
 *        |                              + [function] clearTimeout
 *        |                              + [function] setInterval
 *        |                              + [function] clearInterval
 *        |                              + [function] addEventListener < - pageScriptCreated
 *        |                                                            < - load
 *        |                                                            < - initLayoutCompleted
 *        |                                                            < - pageChanged
 *        |                                                            < - contentsSizeChanged
 *        |                                                            < - iconChanged
 *        |                                                            < - loadStarted
 *        |                                                            < - titleChanged
 *        |                                                            < - urlChanged
 *        |                                                            < - repaint
 *        |                                                            < - pageRectChanged
 *        |                                                            < - loadProgress
 *        |                                                            < - message
 *        |                                                            < - firstPaintFinished
 *        |                                                            < - firstScreenFinished
 *        |                                                            < - requestStart
 *        |                                                            < - requestFinished
 *        |                                                            < - consoleMessage
 *        |                                                            < - alert
 *        |                                                            < - confirm
 *        |                                                            < - propmt
 *        |                                                            < - print
 *        |                                                            < - close
 *        |                                                            < - scroll
 *        |                                                            < - selectionChanged
 *        |                                                            < - statusBarMessage
 *        |                                                            < - DOMContentLoaded
 *        |                              + [function] on <- addEventListener alias
 *        |                              + [function] removeEventListener
 *        |                              + [function] off <- removeEventListener alias
 *        |                              + [function] once
 *        |                              + [function] removeAllEventListener
 *        |                              + [function] offAll <- removeAllEventListener alias
 *        |                              + [function] elementRects
 *        |                              + [function] saveImage  <-  * JPG/JPEG/PNG/BMP/PPM/TIFF;
 *        |                              + [function] dataURIFromRect
 *        |                              + [function] savePdf
 *        |                              + [function] sendMouseEvent + click/mousedown/mouseup/mousemove
 *        |                                                          + and key shift/alt/ctrl
 *        |                              + [function] viewport
 *        |                              + [function] setViewport
 *        |                              + [function] contentRect
 *        |                              + [function] netListener
 *        |                              + [function] cookiesFromUrl
 *        |                              + [function] setCookiesFromUrl
 *        |                              + [function] cookieObject
 *        |                              + [function] setCookie
 *        |                              + [function] removeCookie
 *        |                              + [function] clearCookie
 *        |                              + [function] userAgent
 *        |                              + [function] defaultUserAgent
 *        |                              + [function] setUserAgent
 *        |                              + [function] useSystemProxy
 *        |                              + [function] setProxy
 *        |                              + [function] clearProxy
 *        |                              + [function] setDetectionRects
 *        |                              + [function] clearDetectionRects
 *        |                              + [function] hasDetectionRects
 *        |                              + [function] setPageZoom
 *        |                              + [function] pageZoom
 *        |                              + [function] setPageScroll
 *        |                              + [function] pageScroll
 *        |                              + [function] pageHTML
 *        |                              + [function] setPageHTML
 *        |                              + [function] pageText
 *        |                              + [function] setUploadFile
 *        |                              + [function] setMaxPagesInCache
 *        |                              + [function] maxPagesInCache
 *        |                              + [function] clearAllPagesInCache
 *        |
 *        |
 *        + [object] console + [function] log
 *                           + [function] dir
 *                           + [function] time
 *                           + [function] timeEnd
 *        + [function] alert  <--referenced from App.alert method
 *        + [function] setTimeout <--referenced from webview.setTimeout method
 *        + [function] clearTimeout <--referenced from webview.clearTimeout method
 *        + [function] setInterval <--referenced from webview.setInterval method
 *        + [function] clearInterval <--referenced from webview.clearInterval method
 *        + [function] print
 *
 * ps: Exposed webview object to the js Engine. It default exposed the following methods and propertys.
 *     which propertys is readonly and methods section need Qt Object.
 *
 *   objectName           windowOpacity             sizeIncrement        customContextMenuRequested(QPoint)
 *   modal                windowModified            baseSize             setEnabled(bool)
 *   windowModality       toolTip                   palette              setDisabled(bool)
 *   enabled              statusTip                 font                 setWindowModified(bool)
 *   geometry             whatsThis                 cursor               setWindowTitle(QString)
 *   frameGeometry        accessibleName            mouseTracking        setStyleSheet(QString)
 *   normalGeometry       accessibleDescription     isActiveWindow       setFocus()
 *   x                    layoutDirection           focusPolicy          update()
 *   y                    autoFillBackground        focus                setVisible(bool)
 *   pos                  styleSheet                contextMenuPolicy    setHidden(bool)
 *   frameSize            locale                    updatesEnabled       show()
 *   size                 windowFilePath            visible              hide()
 *   width                inputMethodHints          minimized            setShown(bool)
 *   height               title                     maximized            showMinimized()
 *   rect                 url                       fullScreen           showMaximized()
 *   childrenRect         icon                      sizeHint             showFullScreen()
 *   childrenRegion       selectedText              minimumSizeHint      showNormal()
 *   sizePolicy           modified                  acceptDrops          raise()
 *   minimumSize          textSizeMultiplier        windowTitle          lower()
 *   maximumSize          zoomFactor                windowIcon           updateMicroFocus()
 *   minimumWidth         renderHints               windowIconText       stop()
 *   minimumHeight        destroyed(QObject*)                            back()
 *   maximumWidth         destroyed()                                    forward()
 *   maximumHeight        deleteLater()                                  reload()
 *
 *
 */

class ScriptBinding : public QObject
{
    Q_OBJECT


private:
    QScriptEngine* engine;
    QScriptValue globalObject;


    void initScriptEngine();
    void initRootSpace();
    void initNativeMethodToRootSpace();
    void initExpandMethodToRootSpace();

    void initConsoleSpace();

    QScriptValue toWrapper(QObject *object);

    // 统一包装 Selector 内部的方法
    QScriptValue wrapperSelector();

public:
    static const QString ROOT;
    static const QString EXPAND_METHOD;
    static const QString CONSOLE;
    static const QString OPEN_FILE_ERROR;

    static FileSystemWatcher* fileSystemWatcher;
    static AppInfo* appInfo;

    ScriptBinding();
    ~ScriptBinding();

    QScriptValue getRootSpace();
    QScriptValue getGlobalObject();
    QScriptEngine* getScriptEngine();
    QScriptValue runScript(const QString code);

    // 此处需要指针类型 否则无法找到静态属性
    static Selector* selector;
    static QWebView* webView;

    // 包裹调用的内置方法需要是静态的
    static QScriptValue wrapperHttpDataArray(QMap<QString, MonitorData*> map, QScriptEngine* engine);
    static QTextCodec* getCodec(QString charset);
    static QString readFile(QString fileName, QTextCodec* charset);

    /* 包裹方法需要是静态的  */
    static QScriptValue isOnline(QScriptContext *context, QScriptEngine *interpreter);

    /* console 相关方法实现 */
    static QMap<QString, qint64> timeMap;
    static QScriptValue consoleLog(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue consoleTime(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue consoleTimeEnd(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue consoleDir(QScriptContext *context, QScriptEngine *interpreter);

    /* JS获取当前所有network数据 */
    static QScriptValue getNetworkData(QScriptContext *context, QScriptEngine *interpreter);

    /* Selector 类需要暴露的方法包裹 */
    static QScriptValue selectorClear(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedIMG(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedPNG(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedGIF(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedICO(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedJPG(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedSVG(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedDoc(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedCSS(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedJS(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedNoneGZip(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedNoneCache(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedNoneCDN(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedCookie(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedTotalTimeout(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedWaitTimeout(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedDownloadTimeout(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedDNSLookupTimeout(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedSizeOut(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedHttp200(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedHttp301(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedHttp302(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedHttp304(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedHttp404(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue selectedFromCDN(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue getSelectedData(QScriptContext *context, QScriptEngine *interpreter);

    /* 其他包裹方法 */
    static QScriptValue process(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue httpRequest(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue loadScript(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue writeFile(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue readFile(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue alert(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue about(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue base64FromFile(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue dataURIFromImage(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue cpu(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue memory(QScriptContext *context, QScriptEngine *interpreter);

    /* 文件观察系统包裹方法  */
    static QScriptValue watchFile(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue unWatcher(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue watcherClose(QScriptContext *context, QScriptEngine *interpreter);
    static QScriptValue watchedFiles(QScriptContext *context, QScriptEngine *interpreter);
    // TODO: ...

signals:

public slots:

};

#endif // SCRIPTBINDING_H
