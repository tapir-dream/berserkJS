#include "monitordata.h"

MonitorData::MonitorData()
{
    StatusCode = 0;
    ReasonPhrase = "";
    FromCache = false;
    RequestURL = "";
    RequestStartTime = 0;
    RequestEndTime = 0;
    ResponseSize = 0;
    ResponseDuration = 0;
    ResponseWaitingDuration = 0;
    ResponseDownloadDuration = 0;
    ResponseDNSLookupDuration = 0;
    ResponseMethod = "";
    UserAgent = "";
    Accept = "";
    AcceptRanges = "";
    Referer = "";
    Age = "";
    AccessControlAllowOrigin = "";
    CacheControl = "";
    Connection = "";
    ContentType= "";
    ContentLength = 0;
    ContentEncoding= "";
    Cookie= "";
    Date = "";
    ETag = "";
    Expires= "";
    IfModifiedSince= "";
    LastModified= "";
    Server = "";
    SetCookie = "";
    P3P = "";
    Vary = "";
    TransferEncoding = "";
    XVia = "";
    XDEBUGIDC = "";
    XPoweredBy = "";
    XCache = "";
    XCacheLookup = "";
    XCacheVarnish = "";
    PoweredByChinaCache = "";
    SINALB = "";
}

bool MonitorData::hasKeepAlive()
{
    return Connection == "Keep-Alive";
}

bool MonitorData::hasGZip()
{
    return ContentEncoding.indexOf("gzip") != -1;
}

bool MonitorData::hasCookie()
{
    return Cookie != "";
}

bool MonitorData::hasCache()
{
    return FromCache;
        /*
            this->httpETag != "" &&
            this->httpLastModified!="" &&
            this->httpIfModifiedSince !="" &&
            this->httpExpires != "";
        */
}

bool MonitorData::hasExpires()
{
    return Expires != "";
}

bool MonitorData::isImgFile()
{
    return ContentType.indexOf("image/") == 0;
}
bool MonitorData::isJpg()
{
    return ContentType.indexOf("image/jpeg") == 0 ||
           ContentType.indexOf("image/jp2") == 0||
           ContentType.indexOf("image/jpm") ==0 ||
           ContentType.indexOf("image/jpx") == 0;
}
bool MonitorData::isGif()
{
    return ContentType.indexOf("image/gif") == 0;
}
bool MonitorData::isPng()
{
    return ContentType.indexOf("image/png") == 0;
}
bool MonitorData::isIco()
{
    return ContentType.indexOf("image/x-icon") == 0;
}
bool MonitorData::isSvg()
{
    return ContentType.indexOf("image/svg+xml") == 0;
}

bool MonitorData::isCssFile()
{
    return ContentType.indexOf("text/css") == 0;
}

bool MonitorData::isJsFile()
{
    return ContentType.indexOf("/javascript") != -1 ||
           ContentType.indexOf("/x-javascript") != -1 ||
           ContentType.indexOf("text/vnd.wap.wmlscript") != -1;
}

bool MonitorData::isDocFile()
{
    return ContentType.indexOf("text/html") != -1 ||
           ContentType.indexOf("application/xhtml+xml") != -1 ||
           ContentType.indexOf("text/vnd.wap.wml") != -1 ||
           ContentType.indexOf("text/xml") != -1;
}

bool MonitorData::isFontFile()
{
    return ContentType.indexOf("application/x-font-") != -1;
}

bool MonitorData::isAudioFile()
{
    return ContentType.indexOf("audio/") == 0;
}

bool MonitorData::isVideoFile()
{
    return ContentType.indexOf("video/") == 0;
}

bool MonitorData::isOtherFile()
{
    return !(isAudioFile() ||
             isCssFile() ||
             isDocFile() ||
             isFontFile() ||
             isImgFile() ||
             isJsFile() ||
             isVideoFile());
}

bool MonitorData::isHttp200()
{
    return StatusCode == 200;
}

bool MonitorData::isHttp301()
{
    return StatusCode == 301;
}

bool MonitorData::isHttp302()
{
    return StatusCode == 302;
}

bool MonitorData::isHttp304()
{
    return StatusCode == 304;
}

bool MonitorData::isHttp404()
{
    return StatusCode == 404;
}

bool MonitorData::isFromCDN()
{
    return XVia.indexOf("(Cdn Cache Server") > 0;
}
