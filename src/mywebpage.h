#ifndef MYWEBPAGE_H
#define MYWEBPAGE_H

/**
 * MyWebPage 类
 * 用于扩展QWebPage类
 *
 * @author Tapir
 * @date   2012-03-21
 */

#include <QtWebKit>

class MyWebPage: public QWebPage
{
    Q_OBJECT
    public:
      MyWebPage();
      QString setUserAgent(const QString & userAgent);
      QString userAgent();
      QString defaultUserAgent;

    protected:
      // Overloaded virtual function reserve.
      // void javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID);
      // bool javaScriptPrompt(QWebFrame* frame, const QString & msg, const QString & defaultValue, QString* result);
      // void javaScriptAlert(QWebFrame* frame, const QString & msg);
      // bool javaScriptConfirm(QWebFrame* frame, const QString & msg);

      QString userAgentForUrl(const QUrl & url) const;

    private:
      QString myUserAgent;

};

#endif // MYWEBPAGE_H
