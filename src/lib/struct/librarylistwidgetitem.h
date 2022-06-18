#ifndef LIBRARYLISTWIDGETITEM_H
#define LIBRARYLISTWIDGETITEM_H
#include <QListWidgetItem>
#include <QApplication>
#include <QFileInfo>
#include "lib/handler/settingshandler.h"
#include "lib/tool/xmath.h"
#include "lib/struct/LibraryListItem27.h"
class LibraryListWidgetItem : public QListWidgetItem
{
public:
    LibraryListWidgetItem(LibraryListItem27 &data, QListWidget* parent = nullptr);
    ~LibraryListWidgetItem();
    LibraryListItem27 getLibraryListItem();
    LibraryListItemType getType();
    static void setSortMode(LibrarySortMode sortMode);
    virtual bool operator< (const QListWidgetItem & other) const override;
    bool operator== (const LibraryListWidgetItem & other) const;
    void dropEvent(QDropEvent* event);
    void updateThumbSize(QSize thumbSize);
    void setThumbFile(QString filePath, QString errorMessage = nullptr);
    virtual LibraryListWidgetItem* clone() const override;
    QSize calculateHintSize(QSize size);
private:
    QString _thumbFile;
    QSize _thumbSize;
    static LibrarySortMode _sortMode;
};

#endif // LIBRARYLISTWIDGETITEM_H
