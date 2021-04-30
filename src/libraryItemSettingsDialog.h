#ifndef LIBRARYLISTITEMSETTINGSDIALOG_H
#define LIBRARYLISTITEMSETTINGSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include "lib/handler/loghandler.h"
#include "lib/handler/settingshandler.h"
#include "lib/struct/LibraryListItemMetaData258.h"

class LibraryItemSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LibraryItemSettingsDialog(QString key, QWidget *parent = nullptr);
    static void getSettings(QWidget *parent, QString key,  bool *ok = nullptr);

private:
    QLabel* offsetLabel;
    QSpinBox* offsetSpinBox;

    static LibraryListItemMetaData258 _libraryListItemMetaData;
    static void showDialog(LibraryItemSettingsDialog *dialog, bool *ok);
};

#endif // LIBRARYLISTITEMSETTINGSDIALOG_H
