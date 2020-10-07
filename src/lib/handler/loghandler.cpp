#include "loghandler.h"

LogHandler::LogHandler()
{

}

void LogHandler::Debug(QString message)
{
    if (_debugMode && !_userDebugMode)
    {
        qDebug() << message;
    }
    else if(_userDebugMode)
    {
        QMutexLocker locker(&mutex);
        userDebugIndex++;
        _debugStore.insert(userDebugIndex, QDateTime::currentDateTime().toString("MM-dd-yyyy_hh-mm-ss-zzz") +" Debug: " + message);
    }
}

void LogHandler::Info(QString message)
{
    if (_debugMode && !_userDebugMode)
    {
        qInfo() << message;
    }
    else if(_userDebugMode)
    {
        QMutexLocker locker(&mutex);
        userDebugIndex++;
        _debugStore.insert(userDebugIndex, QDateTime::currentDateTime().toString("MM-dd-yyyy_hh-mm-ss-zzz") +" Info: " + message);
    }
}

void LogHandler::Warn(QString message)
{
    if (_debugMode && !_userDebugMode)
    {
        qWarning() << message;
    }
    else if(_userDebugMode)
    {
        QMutexLocker locker(&mutex);
        userDebugIndex++;
        _debugStore.insert(userDebugIndex, QDateTime::currentDateTime().toString("MM-dd-yyyy_hh-mm-ss-zzz") +" WARNING: " + message);
    }
}

void LogHandler::Error(QString message)
{
    if(!_userDebugMode)
    {
        QMutexLocker locker(&mutex);
        userDebugIndex++;
        _debugStore.insert(userDebugIndex, QDateTime::currentDateTime().toString("MM-dd-yyyy_hh-mm-ss-zzz") +" ERROR: " + message);
        return;
    }
    qCritical() << message;
}

void LogHandler::Dialog(QString message, XLogLevel level)
{
    Debug(message);
    QMessageBox messageBox;
    messageBox.critical(0, LogHandler::getLevel(level), message);
    messageBox.setFixedSize(500,200);
}

QString LogHandler::getLevel(XLogLevel level)
{
    switch(level)
    {
        case XLogLevel::Debuging:
            return "Debug";
        case XLogLevel::Information:
            return "Information";
        case XLogLevel::Warning:
            return "Warning";
        case XLogLevel::Critical:
            return "Error";
    }

}

void LogHandler::DebugHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QString txt;
    switch (type) {
        case QtInfoMsg:
            txt = QString("Info: %1").arg(msg);
            break;
        case QtDebugMsg:
            txt = QString("Debug: %1").arg(msg);
            break;
        case QtWarningMsg:
            txt = QString("Warning: %1").arg(msg);
        break;
        case QtCriticalMsg:
            txt = QString("Critical: %1").arg(msg);
        break;
        case QtFatalMsg:
            txt = QString("Fatal: %1").arg(msg);
        break;
    }
    QMutexLocker locker(&mutex);
    userDebugIndex++;
    _debugStore.insert(userDebugIndex, QDateTime::currentDateTime().toString("MM-dd-yyyy_hh-mm-ss-zzz") + " " + txt);
}

void LogHandler::UserDebug(bool on)
{
    QMutexLocker locker(&mutex);
    _userDebugMode = on;
    if (on)
    {
        qInstallMessageHandler(DebugHandler);
    }
    else
    {
        qInstallMessageHandler(0);
    }
}

void LogHandler::ExportDebug()
{
    QMutexLocker locker(&mutex);
    if(_debugStore.keys().length() > 0)
    {
        _debugFileName = "Debug_"+QDateTime::currentDateTime().toString("MM-dd-yyyy_hh-mm-ss")+".txt";
        QFile outFile(QApplication::applicationDirPath() + "/" +_debugFileName);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        QMap<qint64, QString> debugStoreMap;
        foreach (qint64 index, _debugStore.keys())
        {
            debugStoreMap.insert(index, _debugStore.value(index));
        }
        foreach (qint64 index, debugStoreMap.keys())
        {
            ts << debugStoreMap.value(index) << Qt::endl;
        }
        userDebugIndex = 0;
        _debugStore.clear();
    }
}

QMutex LogHandler::mutex;
qint64 LogHandler::userDebugIndex = 0;
QHash<qint64, QString> LogHandler::_debugStore;
QString LogHandler::_debugFileName;
bool LogHandler::_userDebugMode = false;
#ifdef QT_DEBUG
    bool LogHandler::_debugMode = true;
#else
    bool LogHandler::_debugMode = false;
#endif
