#ifndef LIBRARYLISTWIDGETITEM_H
#define LIBRARYLISTWIDGETITEM_H
#include <QListWidgetItem>
#include "LibraryListItem.h"
enum LibrarySortMode {
    NAME_ASC,
    NAME_DESC,
    RANDOM,
    CREATED_ASC,
    CREATED_DESC,
    TYPE_ASC,
    TYPE_DESC
};
class LibraryListWidgetItem : public QListWidgetItem
{
public:
    LibraryListWidgetItem(LibraryListItem data);
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
