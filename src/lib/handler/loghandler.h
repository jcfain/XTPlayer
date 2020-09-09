#ifndef LOGHANDLER_H
#define LOGHANDLER_H
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <QMutex>
#include <QFile>
#include <QDateTime>
#include <QApplication>

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
    static void UserDebug(bool on);

private:
    LogHandler();
    static void DebugToFile(QtMsgType type, const QMessageLogContext &, const QString & msg);
    static QMutex mutex;
    static QString _debugFileName;
    static bool _debugMode;

};

#endif // LOGHANDLER_H
