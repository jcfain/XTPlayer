#include "librarylistviewmodel.h"

LibraryListViewModel::LibraryListViewModel(MediaLibraryHandler* mediaLibraryHandler, QObject *parent)
    : QAbstractListModel(parent)
{
    _mediaLibraryHandler = mediaLibraryHandler;
    //connect(_mediaLibraryHandler, &MediaLibraryHandler::libraryLoaded, this,  [this]() { populate(); } );
    connect(_mediaLibraryHandler, &MediaLibraryHandler::itemUpdated, this,   [this](LibraryListItem27 item) {
        beginResetModel();
        endResetModel();
    } );
}

int LibraryListViewModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return getData().size();
}
LibraryListItem27 LibraryListViewModel::getItem(const QModelIndex &index) {
    return index.data(Qt::UserRole).value<LibraryListItem27>();
}
LibraryListItem27 LibraryListViewModel::getItem(int index) {
    return this->index(index, 0).data(Qt::UserRole).value<LibraryListItem27>();
}

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
QList<LibraryListItem27> LibraryListViewModel::getData() const {
    return _mediaLibraryHandler->getLibraryCache();
}
QVariant LibraryListViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() == 0) {
        auto data =  getData();
        auto item = data.value(index.row());
        if (role == Qt::DisplayRole)
        {
                return (item.isMFS ? "(MFS) " : "") + item.nameNoExtension;
        }
        else if(Qt::DecorationRole == role)
        {
            auto thumbInt = SettingsHandler::getThumbSize();
            QSize thumbSize = {thumbInt, thumbInt};
            if(item.thumbState == ThumbState::Waiting)
                return ImageFactory::resizeCache(item.thumbFileLoading, item.ID, thumbSize);
            else if(item.thumbState == ThumbState::Loading)
                return ImageFactory::resizeCache(item.thumbFileLoadingCurrent, item.ID, thumbSize);
            else if(item.thumbState == ThumbState::Error)
                return ImageFactory::resizeCache(item.thumbFileError, item.ID, thumbSize);
            return ImageFactory::resizeCache(item.thumbFile, item.ID, thumbSize);
        }
        else if(role == Qt::UserRole)
        {
            return QVariant::fromValue(item);
        }
        else if (role == Qt::ToolTipRole)
        {
            return item.toolTip;
        }
        else if (item.isMFS && role == Qt::ForegroundRole)
        {
            return QColor(Qt::green);
        }
        else if (item.isMFS && role == Qt::FontRole)
        {
            QFont font;
            font.setBold(true);
            return font;
        }
    }

    return QVariant();
}


