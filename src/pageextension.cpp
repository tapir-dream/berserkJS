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

// 一个内置方法，用户不应该使用它
// 它的作用是将页面所有的一些事件传递到 webview 类中
// 这样让 webview 也可以触发指定信号并响应在 APP 代码环境下的监听内容
void PageExtension::sendSignal(QString signal, QString value)
{
    if(signal.isNull() || signal.isEmpty()) {
        return;
    }
    webview->sendSignal(signal, value);
}

double PageExtension::cpu()
{
    return this->appInfo->getCPU();
}

double PageExtension::memory()
{
    return this->appInfo->getMemory();
}


