#include "cookiejar.h"

CookieJar::CookieJar(QObject *parent) :
    QNetworkCookieJar(parent)
{
}

QList<QNetworkCookie> CookieJar::all() {
    return allCookies();
}
void CookieJar::set(const QList<QNetworkCookie> &cookieList)
{
    setAllCookies(cookieList);
}
