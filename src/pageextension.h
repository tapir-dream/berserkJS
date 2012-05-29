#ifndef PAGEEXTENSION_H
#define PAGEEXTENSION_H

#include <QObject>
#include <QtScript>
#include "scriptbinding.h"
#include "mywebview.h"

class PageExtension : public QObject
{
    Q_OBJECT
private:
    MyWebView* webview;

public:
    explicit PageExtension(MyWebView* webview);

signals:

public slots:
    void postMessage(QString wparam  ="", QString lparam  = "");
};

#endif // PAGEEXTENSION_H
