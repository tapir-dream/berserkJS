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
      QString uploadFile;

    protected:
      // Overloaded virtual function reserve.
      void javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID);
      bool javaScriptPrompt(QWebFrame* frame, const QString & msg, const QString & defaultValue, QString* result);
      void javaScriptAlert(QWebFrame* frame, const QString & msg);
      bool javaScriptConfirm(QWebFrame* frame, const QString & msg);
      bool extension(Extension extension, const ExtensionOption* option, ExtensionReturn* output);
      bool supportsExtension(Extension extension) const;
      QString chooseFile(QWebFrame *originatingFrame, const QString &oldFile);

      QString userAgentForUrl(const QUrl & url) const;

    private:
      QString myUserAgent;

    signals:
        void pageConsoleMessage(QString message, int lineNumber, QString sourceID);
        void pagePrompt(QString msg, QString defaultValue);
        void pageConfirm(QString msg);
        void pageAlert(QString msg);
};

#endif // MYWEBPAGE_H
