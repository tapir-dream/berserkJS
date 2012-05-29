#ifndef FILESYSTEMWATCHER_H
#define FILESYSTEMWATCHER_H

#include <QObject>
#include <QtScript>
#include <QFileSystemWatcher>

class FileSystemWatcher : public QObject
{
    Q_OBJECT
public:

    FileSystemWatcher();

    bool addWatchEvent(QScriptEngine* engine, QScriptValue scriptFunc);
    bool removeWatchedEvent(QScriptValue scriptFunc);
    void addWatchPath(QString path);
    void removeWatchedPath(QString path);
    void removeAllWatchedPaths();
    void removeAllWatchedEvents();
    void watcherClose();
    QStringList watchedPathList();

private:
    typedef struct {
        QScriptValue func;
        QScriptValue activationObject;
        QScriptValue thisObject;
    } ContextInfo;

    bool hasFileSystemWatcher;
    QList<ContextInfo> changeListFunc;
    QFileSystemWatcher* watcher;
    void eventBinding();
    void fireEvents(QString path, QString type);

signals:

public slots:
    void onDirectoryChanged (QString path);
    void onFileChanged (QString path);
    
};

#endif // FILESYSTEMWATCHER_H
