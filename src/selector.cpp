#include "selector.h"

Selector::Selector()
{
    monitorDataMapHandle = MonitorDataMap::getMonitorDataMap();
}

void Selector::clear()
{
    selectedMonitorDataMap.clear();
}

void Selector::addToSelectedMonitorDataMap(selectType type, qint32 value)
{
    // 指定源
    QMap<QString, MonitorData*> map;

    // 当前源
    QMap<QString, MonitorData*> currentSelectedMap;

    int len = selectedMonitorDataMap.keys().size();
    // 如果第一次选择内容，则从总数据源中筛选
    // 当选择数据源中有内容后，从选择源中继续筛选
    map = (len == 0) ? monitorDataMapHandle->getData() : selectedMonitorDataMap;

    QList<QString> keys = map.keys();
    // 获取数据源长度
    int c = map.keys().size();
    // 从指定源中筛选到当前选择集
    for(int i = 0; i < c; ++i) {
        switch(type){
        case IMG:
            if(map[keys.at(i)]->isImgFile()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case PNG:
            if(map[keys.at(i)]->isPng()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case GIF:
            if(map[keys.at(i)]->isGif()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case ICO:
            if(map[keys.at(i)]->isIco()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case JPG:
            if(map[keys.at(i)]->isJpg()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case SVG:
            if(map[keys.at(i)]->isSvg()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case Doc:
            if(map[keys.at(i)]->isDocFile()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case CSS:
            if(map[keys.at(i)]->isCssFile()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case JS:
            if(map[keys.at(i)]->isJsFile()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case Http200:
            if(map[keys.at(i)]->isHttp200()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case Http301:
            if(map[keys.at(i)]->isHttp301()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case Http302:
            if(map[keys.at(i)]->isHttp302()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case Http304:
            if(map[keys.at(i)]->isHttp304()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case Http404:
            if(map[keys.at(i)]->isHttp404()){
                currentSelectedMap.insert(keys.at(i), map[keys.at(i)]);
            }
            break;
        case TotalTimeout:
            if (map[keys.at(i)]->ResponseDuration >= value) {
                currentSelectedMap.insert(keys.at(i),  map[keys.at(i)]);
            }
            break;

        case WaitTimeout:
            if (map[keys.at(i)]->ResponseWaitingDuration >= value) {
                currentSelectedMap.insert(keys.at(i),  map[keys.at(i)]);
            }
            break;
        case DownloadTimeout:
            if (map[keys.at(i)]->ResponseDownloadDuration >= value) {
                currentSelectedMap.insert(keys.at(i),  map[keys.at(i)]);
            }
            break;
        case DNSLookupTimeout:
            if (map[keys.at(i)]->ResponseDNSLookupDuration >= value) {
                currentSelectedMap.insert(keys.at(i),  map[keys.at(i)]);
            }
            break;
        case SizeOut:
            if (map[keys.at(i)]->ResponseSize >= value) {
                currentSelectedMap.insert(keys.at(i),  map[keys.at(i)]);
            }
            break;
        case FromCDN:
            if (map[keys.at(i)]->isFromCDN()) {
                currentSelectedMap.insert(keys.at(i),  map[keys.at(i)]);
            }
            break;
        case Cookie:
            if (map[keys.at(i)]->hasCookie()) {
                currentSelectedMap.insert(keys.at(i),  map[keys.at(i)]);
            }
            break;
       case NoneGZip:
            if (!map[keys.at(i)]->hasGZip()) {
                currentSelectedMap.insert(keys.at(i),  map[keys.at(i)]);
            }
            break;
        case NoneCache:
             if (!map[keys.at(i)]->hasCache()) {
                 currentSelectedMap.insert(keys.at(i),  map[keys.at(i)]);
             }
             break;
        case NoneCND:
             if (!map[keys.at(i)]->isFromCDN()) {
                 currentSelectedMap.insert(keys.at(i),  map[keys.at(i)]);
             }
             break;
        }
    }
    // 清空上次选择源，修正为本次积累选择内容
    selectedMonitorDataMap = currentSelectedMap;
}
void Selector::selectedTotalTimeout(qint32 value)
{
    addToSelectedMonitorDataMap(TotalTimeout, value);
}

void Selector::selectedWaitTimeout(qint32 value)
{
    addToSelectedMonitorDataMap(WaitTimeout, value);
}

void Selector::selectedDownloadTimeout(qint32 value)
{
    addToSelectedMonitorDataMap(DownloadTimeout, value);
}

void Selector::selectedDNSLookupTimeout(qint32 value)
{
    addToSelectedMonitorDataMap(DNSLookupTimeout, value);
}

void Selector::selectedSizeOut(qint32 value)
{
    addToSelectedMonitorDataMap(SizeOut,value);
}

void Selector::selectedCookie()
{
    addToSelectedMonitorDataMap(Cookie);
}

void Selector::selectedNoneGZip()
{
    addToSelectedMonitorDataMap(NoneGZip);
}

void Selector::selectedNoneCache()
{
    addToSelectedMonitorDataMap(NoneCache);
}

void Selector::selectedNoneCDN()
{
    addToSelectedMonitorDataMap(NoneCND);
}

void Selector::selectedIMG()
{
    addToSelectedMonitorDataMap(IMG);
}
void Selector::selectedPNG()
{
    addToSelectedMonitorDataMap(PNG);
}
void Selector::selectedGIF()
{
    addToSelectedMonitorDataMap(GIF);
}
void Selector::selectedICO()
{
    addToSelectedMonitorDataMap(ICO);
}
void Selector::selectedJPG()
{
    addToSelectedMonitorDataMap(JPG);
}
void Selector::selectedSVG()
{
    addToSelectedMonitorDataMap(SVG);
}
void Selector::selectedDoc()
{
    addToSelectedMonitorDataMap(Doc);
}
void Selector::selectedCSS()
{
    addToSelectedMonitorDataMap(CSS);
}
void Selector::selectedJS()
{
    addToSelectedMonitorDataMap(JS);
}

void Selector::selectedHttp200()
{
    addToSelectedMonitorDataMap(Http200);
}
void Selector::selectedHttp301()
{
    addToSelectedMonitorDataMap(Http301);
}
void Selector::selectedHttp302()
{
    addToSelectedMonitorDataMap(Http302);
}
void Selector::selectedHttp304()
{
    addToSelectedMonitorDataMap(Http304);
}
void Selector::selectedHttp404()
{
    addToSelectedMonitorDataMap(Http404);
}

void Selector::selectedFromCDN()
{
    addToSelectedMonitorDataMap(FromCDN);
}

QMap<QString, MonitorData*> Selector::get()
{
    return selectedMonitorDataMap;
}
