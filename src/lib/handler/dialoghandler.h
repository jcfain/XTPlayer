#ifndef DIALOGHANDLER_H
#define DIALOGHANDLER_H

#include <QObject>
#include <QDialog>
#include <QGridLayout>
#include <QMovie>
#include <QLabel>
#include <QMessageBox>
#include "lib/handler/loghandler.h"

class DialogHandler : public QObject
{
    Q_OBJECT
public:
    explicit DialogHandler(QObject *parent = nullptr);
    static void Dialog(QWidget* parent, QString message, XLogLevel level);
    static void Loading(QWidget* parent, QString message);
    static void LoadingClose();

private:
    static QDialog* _loadingWidget;
};

#endif // DIALOGHANDLER_H
