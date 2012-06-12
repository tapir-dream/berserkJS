#include "pageextension.h"


PageExtension::PageExtension(MyWebView* webview)
{
    this->webview = webview;
    this->getCPUPercentage = new GetCPUPercentage();
}

PageExtension::~PageExtension()
{
    delete webview;
    delete getCPUPercentage;
}

void PageExtension::postMessage(QString wparam , QString lparam)
{
    webview->sendPageMessage(wparam, lparam);
}

double PageExtension::cpu()
{
    return this->getCPUPercentage->get();
}


