#include "pageextension.h"


PageExtension::PageExtension(MyWebView* webview)
{
    this->webview = webview;
    this->appInfo = new AppInfo();
}

PageExtension::~PageExtension()
{
    delete webview;
    delete appInfo;
}

void PageExtension::message(QString wparam, QString lparam)
{
    webview->sendPageMessage(wparam, lparam);
}

double PageExtension::cpu()
{
    return this->appInfo->getCPU();
}

double PageExtension::memory()
{
    return this->appInfo->getMemory();
}


