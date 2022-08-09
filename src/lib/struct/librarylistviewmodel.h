#ifndef LIBRARYLISTVIEWMODEL_H
#define LIBRARYLISTVIEWMODEL_H

#include <QAbstractListModel>
#include <QPixmap>
#include "lib/struct/LibraryListItem27.h"
#include "lib/handler/settingshandler.h"
#include "lib/handler/medialibraryhandler.h"
#include "lib/tool/imagefactory.h"

class LibraryListViewModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
       SortRole=Qt::UserRole
    };
    explicit LibraryListViewModel(MediaLibraryHandler* mediaLibraryHandler, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

//    // Fetch data dynamically:
//    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

//    bool canFetchMore(const QModelIndex &parent) const override;
//    void fetchMore(const QModelIndex &parent) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void clear();
    LibraryListItem27 getItem(const QModelIndex &index);
    LibraryListItem27 getItem(int index);
    void addItemFront(LibraryListItem27 list);

    void populate(QList<LibraryListItem27> list);
    void populate();
    void setSortMode(LibrarySortMode sortMode);

//    // Add data:
//    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

//    // Remove data:
//    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;


private:
    QList<LibraryListItem27> mDatas;
    MediaLibraryHandler* _mediaLibraryHandler;

};

#endif // LIBRARYLISTVIEWMODEL_H
