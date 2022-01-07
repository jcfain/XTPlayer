#ifndef LIBRARYLISTWIDGETITEM_H
#define LIBRARYLISTWIDGETITEM_H
#include <QListWidgetItem>
#include "LibraryListItem27.h"
#include <QApplication>
#include <QFileInfo>
#include "../handler/settingshandler.h"
#include "../tool/xmath.h"
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
