#ifndef LIBRARYLISTWIDGETITEM_H
#define LIBRARYLISTWIDGETITEM_H
#include <QListWidgetItem>
#include "LibraryListItem.h"
enum LibrarySortMode {
    DEFAULT,
    RANDOM,
    MODIFIED_ASC,
    MODIFIED_DESC
};
class LibraryListWidgetItem : public QListWidgetItem
{
public:
    LibraryListWidgetItem(LibraryListItem data);
    LibraryListItem getLibraryListItem();
    static void setSortMode(LibrarySortMode sortMode);
    virtual bool operator< (const QListWidgetItem & other) const override;
private:
    static LibrarySortMode _sortMode;
};

#endif // LIBRARYLISTWIDGETITEM_H
