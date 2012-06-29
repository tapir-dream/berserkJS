#ifndef PAGEEXTENSION_H
#define PAGEEXTENSION_H

#include <QObject>
#include <QtScript>
#include "scriptbinding.h"
#include "mywebview.h"
#include "appinfo.h"

class PageExtension : public QObject
{
    Q_OBJECT
private:
    MyWebView* webview;

public:
    explicit PageExtension(MyWebView* webview);
    ~PageExtension();
    AppInfo* appInfo;
signals:

public slots:
    void message(QString wparam = "",
                 QString lparam = "");
    double cpu();
    double memory();
};

#endif // PAGEEXTENSION_H
