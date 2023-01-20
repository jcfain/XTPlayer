#ifndef PLAYLISTVIEWMODEL_H
#define PLAYLISTVIEWMODEL_H

#include <QAbstractListModel>
#include <QPixmap>
#include <QColor>
#include <QFont>
#include <QDragMoveEvent>
#include "librarylistviewmodel.h"
#include "lib/struct/LibraryListItem27.h"
#include "lib/handler/settingshandler.h"
#include "lib/handler/medialibraryhandler.h"
#include "lib/tool/imagefactory.h"

class PlaylistViewModel : public LibraryListViewModel
{
    Q_OBJECT

public:
    explicit PlaylistViewModel(MediaLibraryHandler* mediaLibraryHandler, QObject *parent = nullptr);

    void populate(QList<LibraryListItem27> list);
    void removeItem(LibraryListItem27 item);
    QList<LibraryListItem27> getPopulated();
    void dePopulate();
    void overRideThumbSize(int width);
    void setDragEnabled(bool enabled);
    void clearOverRideThumbSize();

    bool setData( const QModelIndex & index, const QVariant & value, int role ) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    Qt::DropActions supportedDragActions() const override { return Qt::MoveAction; }
    Qt::DropActions supportedDropActions() const override { return Qt::MoveAction; }
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
protected:
    QList<LibraryListItem27> getData() const override;

private:
    bool _resetThumbSize = false;
    bool _dragEnabled = false;
    QList<LibraryListItem27> _data;
};

#endif // PLAYLISTVIEWMODEL_H
