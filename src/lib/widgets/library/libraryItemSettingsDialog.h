#ifndef LIBRARYLISTITEMSETTINGSDIALOG_H
#define LIBRARYLISTITEMSETTINGSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include "lib/struct/LibraryListItemMetaData258.h"
#include "lib/struct/LibraryListItem27.h"

class LibraryItemSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LibraryItemSettingsDialog(QWidget *parent = nullptr);
    static void getSettings(QWidget *parent, const LibraryListItem27 item,  bool *ok = nullptr);

private:
    QLabel* offsetLabel;
    QSpinBox* offsetSpinBox;
    QLabel* globalOffsetLabel;
    QLabel* globalOffsetValueLabel;
    QLabel* smartOffsetValueLabel;
    QSpinBox* globalOffsetSpinBox;


    QLabel* moneyShotLabel;
    QLineEdit* moneyShotLineEdit;
    QPushButton* resetMoneyShotButton;

    static LibraryListItemMetaData258 _libraryListItemMetaData;
    static void showDialog(LibraryItemSettingsDialog *dialog, bool *ok);
    void updateOffsetLabel();
};

#endif // LIBRARYLISTITEMSETTINGSDIALOG_H
