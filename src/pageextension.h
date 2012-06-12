#ifndef PAGEEXTENSION_H
#define PAGEEXTENSION_H

#include <QObject>
#include <QtScript>
#include "scriptbinding.h"
#include "mywebview.h"
#include "getcpupercentage.h"

class PageExtension : public QObject
{
    Q_OBJECT
private:
    MyWebView* webview;

public:
    explicit PageExtension(MyWebView* webview);
    ~PageExtension();
    GetCPUPercentage* getCPUPercentage;
signals:

public slots:
    void postMessage(QString wparam  ="", QString lparam  = "");
    double cpu();
};

#endif // PAGEEXTENSION_H
