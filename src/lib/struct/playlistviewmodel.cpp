#include "playlistviewmodel.h"

PlaylistViewModel::PlaylistViewModel(MediaLibraryHandler* mediaLibraryHandler, QObject *parent)
    : LibraryListViewModel(mediaLibraryHandler, parent)
{
}

// override protected
QList<LibraryListItem27> PlaylistViewModel::getData() const {
    return _data;
}

QList<LibraryListItem27> PlaylistViewModel::getPopulated() {
    return _data;
}
void PlaylistViewModel::populate(QList<LibraryListItem27> list) {
    beginResetModel();
    _data = list;
    m_librarySize = _data.size();
    endResetModel();
}
void PlaylistViewModel::dePopulate() {
    beginResetModel();
    _data.clear();
    m_librarySize = _data.size();
    endResetModel();
}

void PlaylistViewModel::removeItem(LibraryListItem27 item) {
    _data.removeOne(item);
    m_librarySize = _data.size();
}
void PlaylistViewModel::overRideThumbSize(int width) {
    overRideThumbSizeWidth = width;
}
void PlaylistViewModel::clearOverRideThumbSize() {
    _resetThumbSize = true;
    overRideThumbSizeWidth = -1;
}

void PlaylistViewModel::setDragEnabled(bool enabled) {
    _dragEnabled = enabled;
}
bool PlaylistViewModel::setData(const QModelIndex & index, const QVariant & value, int role )
{
    if (!_data.isEmpty() && index.isValid() && role == Qt::EditRole) {

        _data.replace(index.row(), value.value<LibraryListItem27>());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool PlaylistViewModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(QModelIndex(), position, position+rows-1);
    for (int row = 0; row < rows; ++row) {
        _data.insert(position, parent.data(Qt::UserRole).value<LibraryListItem27>());
    }
    endInsertRows();
    return true;
}

bool PlaylistViewModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        _data.removeAt(position);
    }
    endRemoveRows();
    return true;
}
Qt::ItemFlags PlaylistViewModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    if(_dragEnabled) {
        if (index.isValid())
            return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
        else
            return Qt::ItemIsDropEnabled | defaultFlags;
    }
    return defaultFlags;
}
QStringList PlaylistViewModel::mimeTypes() const
{
    QStringList types;
    types << "application/library.list.item.model";
    return types;
}

QMimeData *PlaylistViewModel::mimeData(const QModelIndexList &indexes) const
 {
     QMimeData *mimeData = new QMimeData();
     QByteArray encodedData;

     QDataStream stream(&encodedData, QIODevice::WriteOnly);

     foreach (const QModelIndex &index, indexes) {
         if (index.isValid()) {
             LibraryListItem27 text = data(index, Qt::UserRole).value<LibraryListItem27>();
             stream << text;
         }
     }

     mimeData->setData("application/library.list.item.model", encodedData);
     return mimeData;
 }

bool PlaylistViewModel::dropMimeData(const QMimeData *data,
    Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    qDebug() << action;
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("application/library.list.item.model"))
        return false;

    if (column > 0)
        return false;
    int beginRow;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
        beginRow = parent.row();
    else
        beginRow = rowCount(QModelIndex());
    QByteArray encodedData = data->data("application/library.list.item.model");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<LibraryListItem27> newItems;
    int rows = 0;

    while (!stream.atEnd()) {
        LibraryListItem27 text;
        stream >> text;
        newItems << text;
        ++rows;
    }
    insertRows(beginRow, rows, QModelIndex());
    foreach (const LibraryListItem27 &text, newItems) {
        QModelIndex idx = index(beginRow, 0, QModelIndex());
        setData(idx, QVariant::fromValue(text), Qt::EditRole);
        beginRow++;
    }

    return true;
}
