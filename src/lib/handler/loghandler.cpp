#include "loghandler.h"

LogHandler::LogHandler()
{

}

void LogHandler::Debug(QString message)
{
    if (_debugMode)
    {
        QMutexLocker locker(&mutex);
        qDebug() << message;
    }
}

void LogHandler::Info(QString message)
{
    if (_debugMode)
    {
        QMutexLocker locker(&mutex);
        qInfo() << message;
    }
}

void LogHandler::Warn(QString message)
{
    if (_debugMode)
    {
        QMutexLocker locker(&mutex);
        qWarning() << message;
    }
}

void LogHandler::Error(QString message)
{
    QMutexLocker locker(&mutex);
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
    QMutexLocker locker(&mutex);
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

void LogHandler::DebugToFile(QtMsgType type, const QMessageLogContext &, const QString & msg)
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
    QFile outFile(QApplication::applicationDirPath() + "/" +_debugFileName);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << Qt::endl;
}

void LogHandler::UserDebug(bool on)
{
    _debugMode = on;
    if (on)
    {
        _debugFileName = "Debug_"+QDateTime::currentDateTime().toString("ddMMyyyy-hhmmss")+".txt";
        qInstallMessageHandler(DebugToFile);
    }
    else
    {
        qInstallMessageHandler(0);
    }
}

QMutex LogHandler::mutex;
QString LogHandler::_debugFileName;
#ifdef QT_DEBUG
    bool LogHandler::_debugMode = true;
#else
    bool LogHandler::_debugMode = false;
#endif
