#include "loghandler.h"
#include <QMessageBox>
#include <QDebug>

LogHandler::LogHandler()
{

}

void LogHandler::Debug(QString message)
{
    qDebug() << message;
}

void LogHandler::Info(QString message)
{
    qInfo() << message;
}

void LogHandler::Warn(QString message)
{
    qWarning() << message;
}

void LogHandler::Error(QString message)
{
    qCritical() << message;
}

void LogHandler::Dialog(QString message, LogLevel level)
{
    QMessageBox messageBox;
    messageBox.critical(0, LogHandler::getLevel(level), message);
    messageBox.setFixedSize(500,200);
}

QString LogHandler::getLevel(LogLevel level)
{
    switch(level) {
        case LogLevel::Debuging:
            return "Debug";
        case LogLevel::Information:
            return "Information";
        case LogLevel::Warning:
            return "Warning";
        case LogLevel::Critical:
            return "Error";
    }

}
