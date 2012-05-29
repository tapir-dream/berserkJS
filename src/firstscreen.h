#ifndef FIRSTSCREEN_H
#define FIRSTSCREEN_H

/**
 * FirstScreen 类
 * 用于处理指定渲染区域内像素判断首屏是否渲染完毕
 *
 * @author Tapir
 * @date   2012-03-30
 */

#include <QObject>
#include <QSize>
#include <QPainter>
#include <QImage>
#include <QPoint>
#include <QList>
#include <QTimer>
#include <QWebPage>
#include <QWebFrame>

class FirstScreen : public QObject
{
    Q_OBJECT
public:
    FirstScreen(QWebPage* webPage) ;
    void setStartScanViewTime(qint64 firstPaintTimeout = 0);
    void setCustomDetectionPoints(QList<QPoint> points, float sameRate = 0.95);
    void close();

private:
    QSize viewportSize;
    QWebFrame* myFrame;
    QWebPage* myWebPage;
    int splitCount;
    int detectionInterval;

    int timerId;
    QTimer* timer;

    bool isCustomDetectoinPointMode;

    // 可接受的像素不同数
    int referenceValue;

    // 最大完成状态校验次数
    int maxFinishedNumber;
    // 当前累积完成状态
    int currentFinishedNumber;
    // 当前完成时间
    qint64 currentFinishedTimeout;

    qint64 firstPaintTimeout;

    qint64 startScanViewTime;
    QList<QPoint> detectionPointList;
    QList<QRgb> detectionRGBList;


    void initScreenSplit();
    void initDetectionRGBList();
    QImage getViewportImage();
    bool scanViewPortRenderFinished();


signals:
    // 使用自定义信号
    void firstScreenRenderFinish(int timeout, QString url);

private slots:
    void onTimeout();

};

#endif // FIRSTSCREEN_H
