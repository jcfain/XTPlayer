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

void LogHandler::Dialog(QString message, XLogLevel level)
{
    QMessageBox messageBox;
    messageBox.critical(0, LogHandler::getLevel(level), message);
    messageBox.setFixedSize(500,200);
}

QString LogHandler::getLevel(XLogLevel level)
{
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
