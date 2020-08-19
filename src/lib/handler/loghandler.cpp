#include "loghandler.h"

LogHandler::LogHandler()
{

}

void LogHandler::Debug(QString message)
{
    QMutexLocker locker(&mutex);
    qDebug() << message;
}

void LogHandler::Info(QString message)
{
    QMutexLocker locker(&mutex);
    qInfo() << message;
}

void LogHandler::Warn(QString message)
{
    QMutexLocker locker(&mutex);
    qWarning() << message;
}

void LogHandler::Error(QString message)
{
    QMutexLocker locker(&mutex);
    qCritical() << message;
}

void LogHandler::Dialog(QString message, XLogLevel level)
{
    //QMutexLocker locker(&mutex);
    QMessageBox messageBox;
    messageBox.critical(0, LogHandler::getLevel(level), message);
    messageBox.setFixedSize(500,200);
}

QString LogHandler::getLevel(XLogLevel level)
{
    QMutexLocker locker(&mutex);
    switch(level) {
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

QMutex LogHandler::mutex;
