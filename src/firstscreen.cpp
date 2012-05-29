#include <QDateTime>
#include <qDebug>
#include "firstscreen.h"

FirstScreen::FirstScreen(QWebPage* webPage)
{
    this->myWebPage = webPage;
    this->myFrame = webPage->mainFrame();
    this->viewportSize = webPage->viewportSize();

    this->startScanViewTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    this->detectionInterval = 250;
    this->maxFinishedNumber = 12;
    this->isCustomDetectoinPointMode = false;
}

void FirstScreen::setStartScanViewTime(qint64 firstPaintTimeout)
{
    QString url = myFrame->url().toString().trimmed();
    // 如果是空页面就不检查首屏了
    if (url.isEmpty() || url.toLower() == "about:blank") {
        // qDebug()<<"empty";
        return;
    }
    this->firstPaintTimeout = firstPaintTimeout;
    this->startScanViewTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    // 非自定义选择点监测时采用均匀分布监测
    if (!isCustomDetectoinPointMode) {
        initScreenSplit();
    }

    initDetectionRGBList();

    this->currentFinishedNumber = 0;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(detectionInterval);
    timerId = timer->timerId();
}

void FirstScreen::onTimeout()
{
    bool currentViewPortRenderFinished = scanViewPortRenderFinished();

    // 没开始完成计数并且首屏像素有变
    if (currentFinishedNumber == 0 && !currentViewPortRenderFinished) {
        currentFinishedNumber++;
        return;
    }

    // 一旦开始计数，如果像素还有变化，则立即清0
    if (currentFinishedNumber > 0 && !currentViewPortRenderFinished) {
        currentFinishedNumber = 0;
        return;
    }

    // 首屏像素没变，则开始计数
    if (currentFinishedNumber >= 0 && currentViewPortRenderFinished) {
        currentFinishedNumber++;
    }

    // 如果达到总数重复检测条件就完成了
    if (currentFinishedNumber >= maxFinishedNumber) {
        int firstScreenRenderTimeout =
                QDateTime::currentDateTime().toMSecsSinceEpoch()
                + firstPaintTimeout - startScanViewTime
                - (detectionInterval * maxFinishedNumber);

        //qDebug()<<"firstScreenRenderTimeout: "<<firstScreenRenderTimeout;
        emit firstScreenRenderFinish(firstScreenRenderTimeout, myFrame->url().toString());
        close();
        return;
    }
}

void FirstScreen::setCustomDetectionPoints(QList<QPoint> points, float sameRate)
{
    detectionPointList = points;
    // 可接受的相同点基数默认为全部监测点的95%
    referenceValue = points.size() * sameRate;
    isCustomDetectoinPointMode = true;
}

void FirstScreen::initScreenSplit()
{
    splitCount = 120;
    // 可接受的相同点基数
    referenceValue = 12400;

    // 每次清空待测点坐标
    detectionPointList.clear();

    // 根据当前视口大小计算出点检测点分布
    QPoint step(viewportSize.width()/splitCount,
                viewportSize.height()/splitCount);

    for (int i = 1; i < splitCount; ++i) {
        for (int j = 1; j < splitCount; ++j) {
            detectionPointList.append(QPoint(step.x()*i, step.y()*j));
        }
    }
}

void FirstScreen::initDetectionRGBList()
{
    // 每次清空待测点色值
    detectionRGBList.clear();

    // 使用白色初始化所有标记点
    QRgb rgb = qRgb(255, 255, 255);
    int points = detectionPointList.size();
    for (int i = 0; i < points; ++i) {
        detectionRGBList.append(rgb);
    }
}

QImage FirstScreen::getViewportImage()
{
    QImage image(viewportSize, QImage::Format_RGB16);
    // transprent background
    image.fill(Qt::transparent);

    // render the web page in QImage Object.
    QPainter *p = new QPainter();
    p->begin(&image);
    // clip to viewport.
    QRegion region(0, 0, viewportSize.width(), viewportSize.height());
    myFrame->render(p, region);
    p->end();
    return image;
}

bool FirstScreen::scanViewPortRenderFinished()
{
    QImage viewportImage = getViewportImage();

    // 实际相同点总数
    int sameCount = 0;
    int detectionPointSize = detectionPointList.size();

    for (int i = 0; i < detectionPointSize; ++i) {
        QPoint detectionPoint = detectionPointList.at(i);
        QRgb detectionPointRGB = detectionRGBList.at(i);
        QRgb currentRGB = viewportImage.pixel(detectionPoint);
        if (currentRGB == detectionPointRGB) {
            ++sameCount;
        }
        detectionRGBList[i] = currentRGB;
    }

    // 检测点 - 实际相同点总数 为实际不同点
    // 如果不同点小于可接受基数，则返回true
    // 说明首屏渲染完毕
    //qDebug()<<sameCount<< referenceValue;

    return sameCount >= referenceValue;
}

void FirstScreen::close()
{
    killTimer(timerId);
}
