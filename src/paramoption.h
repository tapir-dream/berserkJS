#ifndef PARAMOPTION_H
#define PARAMOPTION_H

#include <QObject>
#include <QMap>

class paramOption : public QObject
{
    Q_OBJECT
public:
    explicit paramOption(QObject *parent);
    QMap<QString, int> map;
    void set(int key, QString value);
    bool checkOption(QString key);

signals:

public slots:

};

#endif // PARAMOPTION_H
