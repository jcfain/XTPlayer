#include "dialoghandler.h"

DialogHandler::DialogHandler(QObject *parent)
    : QObject{parent}
{

}

void DialogHandler::Dialog(QWidget* parent, QString message, XLogLevel level)
{
    LogHandler::Debug(message);
    QMessageBox messageBox(parent);
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

void DialogHandler::Loading(QWidget* parent, QString message)
{
    LogHandler::Debug(message);
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

void DialogHandler::LoadingClose()
{
    LogHandler::Debug("Close Loading");
    _loadingWidget->close();
    delete _loadingWidget;
}
QDialog* DialogHandler::_loadingWidget;
