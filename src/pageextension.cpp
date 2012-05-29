#include "pageextension.h"

PageExtension::PageExtension(MyWebView* webview)
{
    this->webview = webview;
}

void PageExtension::postMessage(QString wparam , QString lparam)
{
    webview->sendPageMessage(wparam, lparam);
}
