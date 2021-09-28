#ifndef LOGHANDLER_H
#define LOGHANDLER_H
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <QMutex>
#include <QFile>
#include <QDateTime>
#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QMovie>


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
    static void ExportDebug();
    static void Loading(QWidget* parent, QString message);
    static void LoadingClose();

private:
    LogHandler();
    static void DebugHandler(QtMsgType type, const QMessageLogContext &, const QString & msg);
    static QMutex mutex;
    static QString _debugFileName;
    static qint64 userDebugIndex;
    static bool _debugMode;
    static bool _userDebugMode;
    static QHash<qint64, QString> _debugStore;
    static QDialog* _loadingWidget;

};

#endif // LOGHANDLER_H
