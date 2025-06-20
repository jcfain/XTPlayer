#include "dialoghandler.h"
#include <QInputDialog>
#include <QDialogButtonBox>

DialogHandler::DialogHandler(QObject *parent)
    : QObject{parent}
{

}

void DialogHandler::MessageBox(QWidget* parent, QString message, XLogLevel level)
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

int DialogHandler::Dialog(QWidget* parent, QLayout* layout, bool modal, bool showAccept)
{
    if(!_dialog) {
        _dialog = new QDialog(parent);
        _dialog->setMinimumSize({175, 100});
        connect(_dialog, &QDialog::destroyed, [](){
            qDebug("Dialog is gone.");
            _dialog = 0;
        });
        //layout->setParent(_dialog);

        if(showAccept) {
            auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                             | QDialogButtonBox::Cancel, _dialog);

            connect(buttonBox, &QDialogButtonBox::accepted, _dialog, &QDialog::accept);
            connect(buttonBox, &QDialogButtonBox::rejected, _dialog, &QDialog::reject);
            connect(buttonBox, &QDialogButtonBox::accepted, DialogAccepted);
            connect(buttonBox, &QDialogButtonBox::rejected, DialogRejected);
            layout->addWidget(buttonBox);
        }
        if(!modal)
            _dialog->setAttribute(Qt::WA_DeleteOnClose);
        else
            _dialog->deleteLater();
        _dialog->setLayout(layout);
        if(modal)
            return _dialog->exec();
        else
            _dialog->show();
    }
    return -1;
}

int DialogHandler::Dialog(QWidget* parent, QString message, bool modal, bool showAccept)
{
    if(!_dialog) {
        _dialog = new QDialog(parent);
        _dialog->setMinimumSize({175, 100});
        connect(_dialog, &QDialog::destroyed, [](){
            qDebug("Dialog accept is gone.");
            _dialog = 0;
        });
        if(!modal)
            _dialog->setAttribute(Qt::WA_DeleteOnClose);
        else
            _dialog->deleteLater();
        QGridLayout* layout = new QGridLayout(_dialog);
        QLabel* messageLabel = new QLabel(_dialog);
        messageLabel->setText(message);
        messageLabel->setTextFormat(Qt::RichText);
        messageLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        messageLabel->setOpenExternalLinks(true);
        layout->addWidget(messageLabel, 0, 0, 1, 4);
        if(showAccept) {
            QPushButton* okButton = new QPushButton(_dialog);
            okButton->setText("Ok");
            layout->addWidget(okButton, 2, 2, 1, 1);
            connect(okButton, &QPushButton::released, _dialog, &QDialog::accept);
            QPushButton* cancelButton = new QPushButton(_dialog);
            cancelButton->setText("Cancel");
            layout->addWidget(cancelButton, 2, 3, 1, 1);
            connect(cancelButton, &QPushButton::released, _dialog, &QDialog::reject);

            connect(_dialog, &QDialog::accepted, DialogAccepted);
        }
        _dialog->setModal(modal);
        _dialog->setLayout(layout);
        connect(_dialog, &QDialog::rejected, DialogRejected);
        if(modal)
            return _dialog->exec();
        else
            _dialog->show();
    }
    return -1;
}
bool DialogHandler::IsDialogOpen() {
  return (_dialog && _dialog->isVisible());
}
void DialogHandler::DialogClose()
{
    if(_dialog) {
//        if(_dialog != nullptr) {
//            disconnect(_dialog, nullptr, nullptr, nullptr);
            _dialog->close();
//            delete _dialog;
//        }
//        _dialog = 0;
    }
}
void DialogHandler::DialogAccepted()
{
    DialogClose();
}
void DialogHandler::DialogRejected()
{
    DialogClose();
}

void DialogHandler::ShowAboutDialog(QWidget* parent, QString XTPVersion, QString XTEVersion, QString selectedTCodeVersion)
{
    //<a href="https://www.vecteezy.com/free-vector/media-player-icons">Media Player Icons Vectors by Vecteezy</a>
    QGridLayout layout;
    QRect windowRect;
    windowRect.setSize({300, 200});
    layout.setGeometry(windowRect);
    QLabel copyright;
    copyright.setText("<b>XTPlayer v"+ XTPVersion
                      + "</b><br><b>XTEngine v" + XTEVersion
                      + "</b><br>" + selectedTCodeVersion
                      + "<br>"
                                                "Copyright 2025 Jason C. Fain<br>"
                                                "Donate: <a href='https://www.patreon.com/Khrull'>https://www.patreon.com/Khrull</a><br>"
                                                "THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND.");
    copyright.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&copyright);
    QLabel sources;
    sources.setText("This software uses libraries from:");
    sources.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&sources);
    QLabel qtInfo;
    qtInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
#if BUILD_QT5
    qtInfo.setText("<b>Qt v5.15.2</b><br>"
                   "Distributed under the terms of LGPLv3 or later.<br>"
                   "Source: <a href='https://github.com/qt/qt5/releases/tag/v5.15.2'>https://github.com/qt/qt5/releases/tag/v5.15.2</a>");
#else
    qtInfo.setText("<b>Qt v6.9.1</b><br>"
                   "Distributed under the terms of LGPLv3 or later.<br>"
                   "Source: <a href='https://github.com/qt/qt5/releases/tag/v6.9.0'>https://github.com/qt/qt5/releases/tag/v6.9.1</a>");
#endif
    qtInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&qtInfo);
    QLabel rangeSliderInfo;
    rangeSliderInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    rangeSliderInfo.setText("<b>Qt-RangeSlider</b><br>"
                      "Copyright (c) 2019 ThisIsClark (MIT)<br>"
                      "Modifications Copyright (c) 2020 Jason C. Fain<br>"
                      "<a href='https://github.com/ThisIsClark/Qt-RangeSlider'>https://github.com/ThisIsClark/Qt-RangeSlider</a>");
    rangeSliderInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&rangeSliderInfo);
    QLabel boolinqInfo;
    boolinqInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    boolinqInfo.setText("<b>boolinq</b><br>"
                      "Copyright (C) 2019 by Anton Bukov (MIT)<br>"
                      "<a href='https://github.com/k06a/boolinq'>https://github.com/k06a/boolinq</a>");
    boolinqInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&boolinqInfo);
    QLabel qtcompressInfo;
    qtcompressInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    qtcompressInfo.setText("<b>qtcompress</b><br>"
                      "Copyright (C) 2013 Digia Plc (LGPL)<br>"
                      "<a href='https://github.com/nezticle/qtcompress'>https://github.com/nezticle/qtcompress</a>");
    qtcompressInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&qtcompressInfo);
    QLabel qthttpServerInfo;
    qthttpServerInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    qthttpServerInfo.setText("<b>HttpServer</b><br>"
                      "Copyright (C) 2019 Addison Elliott (MIT)<br>"
                      "<a href='https://github.com/addisonElliott/HttpServer'>https://github.com/addisonElliott/HttpServer</a>");
    qthttpServerInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&qthttpServerInfo);
    Dialog(parent, &layout);
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
    if(_loadingWidget) {
        LogHandler::Debug("Close Loading");
        if(_loadingWidget != nullptr) {
            _loadingWidget->close();
            delete _loadingWidget;
        }
        _loadingWidget = 0;
    }
}

PasswordResponse DialogHandler::checkPass(QWidget* parent, QString currentPassword)
{
     bool ok;
     QString text = QInputDialog::getText(parent, tr("STOP!"),
                                          tr("Password:"), QLineEdit::Password,
                                          "", &ok);
     if (ok && !text.isEmpty())
     {
         return CryptHandler::checkPass(text, currentPassword);
     }
     return PasswordResponse::CANCEL ;
}

QString DialogHandler::passwordSetWizard(QWidget* parent, QString currenthashedPass, bool* ok) {
    if(currenthashedPass.isEmpty())
    {
         QString text = QInputDialog::getText(parent, parent->tr("Set password"),
                                              parent->tr("Enter password:"), QLineEdit::PasswordEchoOnEdit,
                                              "", ok);
         if(*ok && !text.isEmpty()) {
             QString text2 = QInputDialog::getText(parent, parent->tr("Set password"),
                                                  parent->tr("Confirm password:"), QLineEdit::PasswordEchoOnEdit,
                                                  "", ok);
             if (*ok && !text.isEmpty() && text == text2)
             {
                 DialogHandler::MessageBox(parent, "Password set.", XLogLevel::Information);
                 return CryptHandler::encryptPass(text);
             } else if(text != text2) {
                 *ok = false;
                 DialogHandler::MessageBox(parent, "Passwords did not match!", XLogLevel::Critical);
             }
         } else {
             *ok = false;
         }
    }
    else
    {
         QString text = QInputDialog::getText(parent, parent->tr("Current password"),
                                              parent->tr("Current password:"), QLineEdit::Password,
                                              "", ok);
         if (*ok && !text.isEmpty())
         {
             if(CryptHandler::checkPass(text, currenthashedPass) == PasswordResponse::CORRECT)
             {
                 QString text = QInputDialog::getText(parent, parent->tr("Change password"),
                                                      parent->tr("New password (Leave blank to remove protection):"), QLineEdit::PasswordEchoOnEdit,
                                                      "", ok);
                 if (*ok)
                 {
                     if(text.isEmpty())
                     {
                        DialogHandler::MessageBox(parent, "Password cleared!", XLogLevel::Information);
                     }
                     else
                     {
                         QString text2 = QInputDialog::getText(parent, parent->tr("Confirm password"),
                                                              parent->tr("Confirm password:"), QLineEdit::PasswordEchoOnEdit,
                                                              "", ok);
                         if (text == text2)
                         {
                             DialogHandler::MessageBox(parent, "Password changed!", XLogLevel::Information);
                             return CryptHandler::encryptPass(text);
                         } else if(text != text2) {
                             *ok = false;
                             DialogHandler::MessageBox(parent, "Passwords did not match!", XLogLevel::Critical);
                         }
                     }
                 }
             }
             else
             {
                 DialogHandler::MessageBox(parent, "Password incorrect!", XLogLevel::Critical);
                 *ok = false;
             }
         }
         else
         {
             *ok = false;
         }
    }
    return nullptr;
}

QDialog* DialogHandler::_loadingWidget = 0;
QDialog* DialogHandler::_dialog = 0;
