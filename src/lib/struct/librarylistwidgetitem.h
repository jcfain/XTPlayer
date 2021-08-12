#ifndef LIBRARYLISTWIDGETITEM_H
#define LIBRARYLISTWIDGETITEM_H
#include <QListWidgetItem>
#include "LibraryListItem.h"
#include <QApplication>
#include <QFileInfo>
#include "../handler/settingshandler.h"
#include "../tool/xmath.h"
class LibraryListWidgetItem : public QListWidgetItem
{
public:
    LibraryListWidgetItem(LibraryListItem &data, QListWidget* parent = nullptr);
    ~LibraryListWidgetItem();
    LibraryListItem getLibraryListItem();
    LibraryListItemType getType();
    bool updateToolTip(LibraryListItem localData);
    static void setSortMode(LibrarySortMode sortMode);
    virtual bool operator< (const QListWidgetItem & other) const override;
    bool operator== (const LibraryListWidgetItem & other) const;
    void dropEvent(QDropEvent* event);
    void updateThumbSize(QSize thumbSize, QString filePath = nullptr);
    virtual LibraryListWidgetItem* clone() const override;
    QSize calculateHintSize(QSize size);
private:
    QString _thumbFile;
    static LibrarySortMode _sortMode;
    QSize calculateMaxSize(QSize size);
};

#endif // LIBRARYLISTWIDGETITEM_H
