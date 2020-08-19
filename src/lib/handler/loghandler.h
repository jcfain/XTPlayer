#ifndef LOGHANDLER_H
#define LOGHANDLER_H
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <QMutex>

enum XLogLevel
{
    Information,
    Debuging,
    Warning,
    Critical
};

class LogHandler
{
public:

    static void Debug(QString message);
    static void Info(QString message);
    static void Error(QString message);
    static void Warn(QString message);
    static void Dialog(QString message, XLogLevel level);
    static QString getLevel(XLogLevel level);

private:
    LogHandler();
    static QMutex mutex;
};

#endif // LOGHANDLER_H
