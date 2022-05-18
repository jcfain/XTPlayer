#include "loghandler.h"

LogHandler::LogHandler()
{

}

void LogHandler::Debug(QString message)
{
    if (_debugMode)
    {
        qDebug() << "XTP DEBUG " + QDateTime::currentDateTime().toString("hh:mm:ss:zzz") +": " +  message;
    }
}

void LogHandler::Info(QString message)
{
    if (_debugMode)
    {
        qInfo() << "XTP INFO " + QDateTime::currentDateTime().toString("hh:mm:ss:zzz") +": " +  message;
    }
}

void LogHandler::Warn(QString message)
{
    if (_debugMode)
    {
        qWarning() << "XTP WARN " + QDateTime::currentDateTime().toString("hh:mm:ss:zzz") + ": " + message;
    }
}

void LogHandler::Error(QString message)
{
    if (_debugMode)
    {
        qCritical() << "XTP ERROR " + QDateTime::currentDateTime().toString("hh:mm:ss:zzz") +": " +  message;
    }
}

void LogHandler::Dialog(QString message, XLogLevel level)
{
    Debug(message);
    QMessageBox messageBox;
    messageBox.setTextFormat(Qt::RichText);
    switch(level)
    {
        case XLogLevel::Debuging:
        break;
        case XLogLevel::Information:
            messageBox.information(0, LogHandler::getLevel(level), message);
        break;
        case XLogLevel::Warning:
            messageBox.warning(0, LogHandler::getLevel(level), message);
        break;
        case XLogLevel::Critical:
            messageBox.critical(0, LogHandler::getLevel(level), message);
        break;
    }
    messageBox.setFixedSize(500,200);
}

void LogHandler::Loading(QWidget* parent, QString message)
{
    Debug(message);
    _loadingWidget = new QDialog(parent);
    _loadingWidget->setModal(true);
    _loadingWidget->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    //_loadingWidget->setAttribute(Qt::WA_TranslucentBackground, false);
    _loadingWidget->setFixedSize(parent->size());
    _loadingWidget->setProperty("cssClass", "loadingSpinner");
    //_loadingWidget->setStyleSheet("* {background-color: rgba(128,128,128, 0.5)}");
    QGridLayout* grid = new QGridLayout(_loadingWidget);
    QMovie* loadingMovie = new QMovie("://images/Eclipse-1s-loading-200px.gif", nullptr, _loadingWidget);
    loadingMovie->setScaledSize({200,200});
    QLabel* loadingLabel = new QLabel(_loadingWidget);
    loadingLabel->setProperty("cssClass", "loadingSpinner");
    loadingLabel->setMovie(loadingMovie);
    loadingLabel->setAlignment(Qt::AlignCenter);
    QLabel* messageLabel = new QLabel(_loadingWidget);
    messageLabel->setText(message);
    grid->addWidget(loadingLabel, 0, 0, Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignBottom );
    grid->addWidget(messageLabel, 1, 0, Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignTop );
    _loadingWidget->hide();
    _loadingWidget->setLayout(grid);
    _loadingWidget->show();
    loadingMovie->start();
}

void LogHandler::LoadingClose()
{
    Debug("Close Loading");
    _loadingWidget->close();
    delete _loadingWidget;
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
        case QtInfoMsg: {
            txt = QString("Info: %1").arg(msg);
            break;
        }
        case QtDebugMsg: {
            txt = QString("Debug: %1").arg(msg);
            break;
        }
        case QtWarningMsg: {
            txt = QString("Warning: %1").arg(msg);
            break;
        }
        case QtCriticalMsg: {
            txt = QString("Critical: %1").arg(msg);
            break;
        }
        case QtFatalMsg: {
            txt = QString("Fatal: %1").arg(msg);
            break;
        }
    }
    QMutexLocker locker(&mutex);
    userDebugIndex++;
    _debugStore.insert(userDebugIndex, QDateTime::currentDateTime().toString("MM-dd-yyyy_hh-mm-ss-zzz") + " " + txt);
}

void LogHandler::setUserDebug(bool on)
{
    QMutexLocker locker(&mutex);
    _debugMode = on;
//    if (on)
//    {
//        qInstallMessageHandler(DebugHandler);
//    }
//    else
//    {
//        qInstallMessageHandler(0);
//    }
}

bool LogHandler::getUserDebug()
{
    QMutexLocker locker(&mutex);
    return _debugMode;
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
QDialog* LogHandler::_loadingWidget;
#ifdef QT_DEBUG
    bool LogHandler::_debugMode = true;
#else
    bool LogHandler::_debugMode = false;
#endif
