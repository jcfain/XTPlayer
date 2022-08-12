#include "librarylistviewmodel.h"

LibraryListViewModel::LibraryListViewModel(MediaLibraryHandler* mediaLibraryHandler, QObject *parent)
    : QAbstractListModel(parent)
{
    _mediaLibraryHandler = mediaLibraryHandler;
    connect(_mediaLibraryHandler, &MediaLibraryHandler::libraryChange, this,  [this]() {
        beginResetModel();
        endResetModel();
    } );
    connect(_mediaLibraryHandler, &MediaLibraryHandler::itemUpdated, this,   [this](LibraryListItem27 item) {
        //beginResetModel();
        auto index = this->index(getData().indexOf(item), 0);
        emit dataChanged(index, index);
        //endResetModel();
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

void LibraryListViewModel::setLibraryViewMode(LibraryView mode) {
    _libraryViewMode = mode;
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
        auto thumbInt = SettingsHandler::getThumbSize();
        if (role == Qt::DisplayRole)
        {
            return (item.isMFS ? "(MFS) " : "") + item.nameNoExtension;
        }
        else if(Qt::DecorationRole == role)
        {
            int scaled = qRound(thumbInt * 0.75);
            QSize thumbSize = {scaled, scaled};
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
        else if (role == Qt::ForegroundRole)
        {
            if (item.type != LibraryListItemType::PlaylistInternal && !QFileInfo::exists(item.script) && !QFileInfo::exists(item.zipFile))
                return QColor(Qt::gray);
            if(item.isMFS)
                return QColor(Qt::green);
        }
        else if (role == Qt::FontRole)
        {
            QFont font;
            if(item.isMFS)
                font.setBold(true);
            if(_libraryViewMode == LibraryView::Thumb)
                font.setPointSizeF((thumbInt * 0.25) * 0.25);
            else
                font.setPointSizeF((thumbInt * 0.25) * 0.35);
            return font;
        }
        else if (role == Qt::SizeHintRole)
        {
            if(_libraryViewMode == LibraryView::Thumb) {
                int scaled  = qRound(thumbInt * 0.25) +thumbInt;
                QSize thumbSizeHint = {scaled, scaled};
                return thumbSizeHint;
            }
        }
        else if (role == Qt::TextAlignmentRole)
        {
            return _libraryViewMode == LibraryView::Thumb ?
                        int(Qt::AlignmentFlag::AlignTop | Qt::AlignmentFlag::AlignHCenter) :
                        int(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);
        }
    }

    return QVariant();
}


