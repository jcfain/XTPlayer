#include "librarylistviewmodel.h"

LibraryListViewModel::LibraryListViewModel(MediaLibraryHandler* mediaLibraryHandler, QObject *parent)
    : QAbstractListModel(parent)
{
    _mediaLibraryHandler = mediaLibraryHandler;
    connect(_mediaLibraryHandler, &MediaLibraryHandler::libraryLoaded, this,  [this]() { populate(); } );
    connect(_mediaLibraryHandler, &MediaLibraryHandler::libraryChange, this,   [this]() { populate(); } );
}

QVariant LibraryListViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
}

int LibraryListViewModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return mDatas.size();
}

void LibraryListViewModel::populate(QList<LibraryListItem27> list) {
    mDatas = list;
}
void LibraryListViewModel::populate()
{
    mDatas = _mediaLibraryHandler->getLibraryCache();
}
void LibraryListViewModel::clear() {
    this->mDatas.clear();
}

LibraryListItem27 LibraryListViewModel::getItem(const QModelIndex &index) {
    return mDatas[index.row()];
}
LibraryListItem27 LibraryListViewModel::getItem(int index) {
    return mDatas[index];
}

void LibraryListViewModel::addItemFront(LibraryListItem27 item) {
    mDatas.push_front(item);
}

//void LibraryListViewModel::update(LibraryListItem27 item) {
//    beginResetModel();
//    int foundItem = -1;
//    foreach (LibraryListItem27 item, mDatas) {
//        if(item.ID == item.ID)
//            mDatas[mDatas.indexOf(item)] = item;
//    }
//}
//bool LibraryListViewModel::hasChildren(const QModelIndex &parent) const
//{
//    // FIXME: Implement me!
//    return false;
//}

//bool LibraryListViewModel::canFetchMore(const QModelIndex &parent) const
//{
//    // FIXME: Implement me!
//    return false;
//}

//void LibraryListViewModel::fetchMore(const QModelIndex &parent)
//{
//    // FIXME: Implement me!
//}

QVariant LibraryListViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if ( role == Qt::DisplayRole)
    {
        if (index.column() == 0)
            return mDatas[index.row()].nameNoExtension;
    }
    else if(Qt::DecorationRole == role)
    {
        auto thumbsize = SettingsHandler::getThumbSize();
        return ImageFactory::resize(mDatas[index.row()].thumbFile, {thumbsize,thumbsize});
    } else if(role == SortRole) {
        auto data = mDatas[index.row()];
        return QVariant::fromValue(data);
    }

    // FIXME: Implement me!
    return QVariant();
}

//bool LibraryListViewModel::insertRows(int row, int count, const QModelIndex &parent)
//{
//    beginInsertRows(parent, row, row + count - 1);
//    // FIXME: Implement me!
//    endInsertRows();
//    return true;
//}

//bool LibraryListViewModel::removeRows(int row, int count, const QModelIndex &parent)
//{
//    beginRemoveRows(parent, row, row + count - 1);
//    // FIXME: Implement me!
//    endRemoveRows();
//    return true;
//}
