#ifndef LIBRARYLISTVIEWMODEL_H
#define LIBRARYLISTVIEWMODEL_H

#include <QAbstractListModel>
#include <QPixmap>
#include <QColor>
#include <QFont>
#include "lib/struct/LibraryListItem27.h"
#include "lib/handler/medialibraryhandler.h"
#include "lib/lookup/enum.h"

class LibraryListViewModel : public QAbstractListModel
{
    Q_OBJECT

signals:

public:
    explicit LibraryListViewModel(MediaLibraryHandler* mediaLibraryHandler, QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    // QModelIndex parent(const QModelIndex &index) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
//    // Fetch data dynamically:
//    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

//    bool canFetchMore(const QModelIndex &parent) const override;
//    void fetchMore(const QModelIndex &parent) override;
    QVariant data(const QModelIndex &index, int role = Qt::EditRole) const override;
    LibraryListItem27 getItem(const QModelIndex &index);
    LibraryListItem27 getItem(int index);

    void setLibraryViewMode(LibraryView mode);

protected:
    virtual const QList<LibraryListItem27>* getData() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    int m_librarySize = 0;
    int overRideThumbSizeWidth = -1;


//    // Add data:
//    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

//    // Remove data:
//    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private slots:

private:
    MediaLibraryHandler* _mediaLibraryHandler;
    LibraryView _libraryViewMode;
    LibrarySortMode _sortMode;

    int getThumbInt() const;


};

#endif // LIBRARYLISTVIEWMODEL_H
