#ifndef DIALOGHANDLER_H
#define DIALOGHANDLER_H

#include <QObject>
#include <QDialog>
#include <QGridLayout>
#include <QMovie>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include "lib/handler/loghandler.h"
#include "lib/handler/crypthandler.h"

class DialogHandler : public QObject
{
    Q_OBJECT
public:
    explicit DialogHandler(QObject *parent = nullptr);
    static void MessageBox(QWidget* parent, QString message, XLogLevel level);
    static int Dialog(QWidget* parent, QLayout* layout, bool modal = true, bool showAccept = false);
    static int Dialog(QWidget* parent, QString message, bool modal = true, bool showAccept = true);
    static bool IsDialogOpen();
    static void DialogClose();
    static void Loading(QWidget* parent, QString message);
    static void LoadingClose();
    static void ShowAboutDialog(QWidget* parent, QString XTPVersion, QString XTEVersion, QString selectedTCodeVersion);
    static PasswordResponse checkPass(QWidget* parent, QString currentPassword);
    static QString passwordSetWizard(QWidget* parent, QString currentPassword, bool* okOut);

private:
    static void DialogAccepted();
    static void DialogRejected();
    static QDialog* _loadingWidget;
    static QDialog* _dialog;
};

#endif // DIALOGHANDLER_H
