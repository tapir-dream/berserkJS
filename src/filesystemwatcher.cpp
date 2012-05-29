#include "filesystemwatcher.h"


FileSystemWatcher::FileSystemWatcher()
{
    hasFileSystemWatcher = false;
    watcher = new QFileSystemWatcher();
}

bool FileSystemWatcher::addWatchEvent(QScriptEngine* engine, QScriptValue scriptFunc)
{
    if (!scriptFunc.isFunction())
        return false;

    // 保留当前运行时所需环境: activationObject/thisObject
    ContextInfo contextInfo;
    contextInfo.activationObject = engine->currentContext()->activationObject();
    contextInfo.thisObject = engine->currentContext()->thisObject();
    contextInfo.func = scriptFunc;
    changeListFunc.append(contextInfo);

    eventBinding();

    return true;
}


bool FileSystemWatcher::removeWatchedEvent(QScriptValue scriptFunc)
{
    if (!scriptFunc.isFunction())
        return false;

    bool isRemoved = false;
    QString funcStr = scriptFunc.toString();
    int c = changeListFunc.size();
    for(int i = 0; i < c; ++i) {
        if (changeListFunc.at(i).func.toString() == funcStr) {
            changeListFunc.removeAt(i);
            isRemoved = true;
        }
    }
    eventBinding();
    return isRemoved;
}

void FileSystemWatcher::removeAllWatchedEvents()
{
    changeListFunc.clear();
    eventBinding();
}

// 避免 QFileSystemWatcher RemovePath 方法 bug
// 将整个对象干掉，重新建立后所有监听目录将自动失效。
void FileSystemWatcher::watcherClose()
{
    changeListFunc.clear();
    eventBinding();
    delete watcher;
    watcher = new QFileSystemWatcher();
}

void FileSystemWatcher::addWatchPath(QString path)
{
    watcher->addPath(path);
}

// Qt QFileSystemWatcher RemovePath 方法有 bug，存在无法移除现象。
void FileSystemWatcher::removeWatchedPath(QString path)
{
    watcher->removePath(path);
}

// Qt QFileSystemWatcher RemovePath 方法有 bug，存在无法移除现象。
void FileSystemWatcher::removeAllWatchedPaths()
{
    QStringList directories = watcher->directories();
    QStringList files = watcher->files();
    if (files.size() > 0) {
        watcher->removePaths(files);
    }

    if (directories.size() > 0) {
        watcher->removePaths(directories);
    }
}

QStringList FileSystemWatcher::watchedPathList()
{
    QStringList list;
    QStringList files = watcher->files();
    QStringList directories = watcher->directories();
    int c;
    c = files.size();
    for (int i = 0; i < c; ++i) {
        list.append(files.at(i));
    }
    c = directories.size();
    for (int i = 0; i < c; ++i) {
        list.append(directories.at(i));
    }
    return list;
}

void FileSystemWatcher::eventBinding()
{
    hasFileSystemWatcher = changeListFunc.size() > 0;

    if (hasFileSystemWatcher) {
        connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(onDirectoryChanged(QString)));
        connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged(QString)));
    } else {
        disconnect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(onDirectoryChanged(QString)));
        disconnect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged(QString)));
    }
}

void FileSystemWatcher::fireEvents(QString path, QString type)
{
    int c = changeListFunc.size();
    if (c > 0) {
        for (int i = 0; i < c; ++i) {
            ContextInfo contextInfo = changeListFunc.at(i);
            contextInfo.func.setScope(contextInfo.activationObject.scope());
            contextInfo.func.call(contextInfo.thisObject,
                                  QScriptValueList() << QScriptValue(path)
                                  << QScriptValue(type));
        }
    }
}

void FileSystemWatcher::onDirectoryChanged(QString path)
{
    fireEvents(path, "[Directory Changed]");
}

void FileSystemWatcher::onFileChanged(QString path)
{
    fireEvents(path, "[File Changed]");
}
