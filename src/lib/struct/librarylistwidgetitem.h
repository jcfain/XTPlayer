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
    LibraryListWidgetItem(LibraryListItem &data);
    LibraryListItem getLibraryListItem();
    LibraryListItemType getType();
    bool updateToolTip(LibraryListItem localData);
    static void setSortMode(LibrarySortMode sortMode);
    virtual bool operator< (const QListWidgetItem & other) const override;
    bool operator== (const LibraryListWidgetItem & other) const;
    void dropEvent(QDropEvent* event);
    virtual LibraryListWidgetItem* clone() const override;
private:
    static LibrarySortMode _sortMode;
};

#endif // LIBRARYLISTWIDGETITEM_H
