#ifndef LIBRARYITEMMETADATADIALOG_H
#define LIBRARYITEMMETADATADIALOG_H

#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QTabWidget>
#include "lib/struct/LibraryListItem27.h"
#include "lib/handler/synchandler.h"

class LibraryItemMetadataDialog : public QDialog
{
    Q_OBJECT
signals:
    void save(LibraryListItemMetaData258 value);
public:
    explicit LibraryItemMetadataDialog(QWidget *parent = nullptr);
    static void getSettings(QWidget *parent, LibraryListItem27* item, SyncHandler* syncHandler, bool *ok = nullptr);

private:
    QTabWidget* m_tabWidget;
    QLabel* offsetLabel;
    QSpinBox* offsetSpinBox;
    QLabel* globalOffsetLabel;
    QLabel* globalOffsetValueLabel;
    QLabel* smartOffsetValueLabel;
    QSpinBox* globalOffsetSpinBox;
    QDoubleSpinBox* funscriptModifierSpinBox;
    QStringList m_itemTagsToAdd;
    QStringList m_itemTagsToRemove;


    QLabel* moneyShotLabel;
    QLineEdit* moneyShotLineEdit;
    QPushButton* resetMoneyShotButton;

    static LibraryListItem27* _libraryListItem;
    static void showDialog(LibraryItemMetadataDialog *dialog, bool *ok);
    void updateOffsetLabel();
    // Builds the read-only Info tab (mirrors the WebUI media item info tab).
    QWidget* buildInfoTab();
    // H:MM:SS (or M:SS under an hour), like the WebUI formatTime().
    static QString formatTime(qint64 ms);
    // "123456 (2:03)" or Unknown for missing/negative values.
    static QString formatOriginalDuration(qint64 ms);
};

#endif // LIBRARYITEMMETADATADIALOG_H
