#ifndef NOMATCHINGSCRIPTDIALOG_H
#define NOMATCHINGSCRIPTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include "lib/handler/loghandler.h"
#include "lib/handler/settingshandler.h"

class NoMatchingScriptDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NoMatchingScriptDialog(QWidget *parent, QString customScript);
    static void show(QWidget *parent, QString customScript);

private:
    QLabel* label;
    QCheckBox* checkbox;

    static void showDialog(NoMatchingScriptDialog *dialog);
};

#endif // NOMATCHINGSCRIPTDIALOG_H
