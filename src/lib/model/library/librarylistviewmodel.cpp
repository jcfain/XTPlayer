#include "librarylistviewmodel.h"

#include "lib/tool/imagefactory.h"
#include "lib/handler/settingshandler.h"

LibraryListViewModel::LibraryListViewModel(MediaLibraryHandler* mediaLibraryHandler, QObject *parent)
    : QAbstractListModel(parent)
{
    _mediaLibraryHandler = mediaLibraryHandler;
    connect(_mediaLibraryHandler, &MediaLibraryHandler::libraryLoaded, this,  [this]() {
        beginResetModel();
        m_librarySize = getData().count();
        QModelIndex topLeft = createIndex(0,0);
        QModelIndex bottomRight = createIndex( m_librarySize, 0);
        emit dataChanged( topLeft, bottomRight );
        endResetModel();
    } );
    connect(_mediaLibraryHandler, &MediaLibraryHandler::metadataProcessEnd, this,  [this]() {
        beginResetModel();
        m_librarySize = getData().count();
        QModelIndex topLeft = createIndex(0,0);
        QModelIndex bottomRight = createIndex( m_librarySize, 0);
        emit dataChanged( topLeft, bottomRight );
        endResetModel();
    } );
    connect(_mediaLibraryHandler, &MediaLibraryHandler::itemUpdated, this,   [this](int indexOfItem, QVector<int> roles) {
        if(indexOfItem > -1 && indexOfItem < m_librarySize) {
            auto index = this->index(indexOfItem, 0);
            emit dataChanged(index, index, roles);
        }
        //endResetModel();
    } );
    connect(_mediaLibraryHandler, &MediaLibraryHandler::itemRemoved, this,   [this](int indexOfItem, int newSize) {
        if(indexOfItem > -1 && indexOfItem < m_librarySize) {
            if(removeRow(indexOfItem))
                m_librarySize = newSize;
        }
    } );
    connect(_mediaLibraryHandler, &MediaLibraryHandler::itemAdded, this,   [this](int indexOfItem, int newSize) {
        if(indexOfItem > -1 && indexOfItem <= m_librarySize)
        {
            m_librarySize = newSize;
            auto index = this->index(indexOfItem, 0);
            //emit dataChanged(index, index);
            beginInsertRows(index, indexOfItem, indexOfItem);
            insertRow(indexOfItem);
            // m_librarySize = newSize;
            // auto index = this->index(indexOfItem, 0);
            // beginInsertRows(QModelIndex(), indexOfItem, indexOfItem);
            // insertRow(indexOfItem);
            endInsertRows();
        }
    } );

}

int LibraryListViewModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_librarySize;
}

QModelIndex LibraryListViewModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column);
}

// QModelIndex LibraryListViewModel::parent(const QModelIndex &index) const
// {
//     return parent;
// }

int LibraryListViewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
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

Qt::ItemFlags LibraryListViewModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    return defaultFlags;
}

QList<LibraryListItem27> LibraryListViewModel::getData() const {
    return _mediaLibraryHandler->getLibraryCache();
}
QVariant LibraryListViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= m_librarySize)
        return QVariant();

    if (index.column() == 0) {
        auto data =  getData();
        auto item = data.value(index.row());
        auto thumbInt = overRideThumbSizeWidth > -1 ? overRideThumbSizeWidth : SettingsHandler::getThumbSize();
        if (role == Qt::DisplayRole)
        {
            return (item.metadata.isMFS ? "(MFS) " : "") + item.nameNoExtension;
        }
        else if(Qt::DecorationRole == role)
        {
            int scaled = qRound(thumbInt * 0.75);
            QSize thumbSize = {scaled, scaled};
            if(item.thumbState == ThumbState::Waiting)
                return ImageFactory::resizeCache(LOADING_IMAGE, item.ID, thumbSize);
            else if(item.thumbState == ThumbState::Loading)
                return ImageFactory::resizeCache(LOADING_CURRENT_IMAGE, item.ID, thumbSize);
            else if(item.thumbState == ThumbState::Error)
                return ImageFactory::resizeCache(ERROR_IMAGE, item.ID, thumbSize);
            return ImageFactory::resizeCache(item.thumbFile, item.ID, thumbSize);
        }
        else if(role == Qt::UserRole)
        {
            return QVariant::fromValue(item);
        }
        else if (role == Qt::ToolTipRole)
        {
            return item.metadata.toolTip;
        }
        else if (role == Qt::ForegroundRole)
        {
            if(item.type != LibraryListItemType::PlaylistInternal)
            {
                if (!item.hasScript && item.metadata.isMFS)
                    return QColor(Qt::GlobalColor::cyan);
                if (!item.hasScript)
                    return QColor(Qt::gray);
                if(item.metadata.isMFS)
                    return QColor(Qt::green);
            }
        }
        else if (role == Qt::BackgroundRole)
        {
            return QColor(Qt::transparent);
        }
        else if (role == Qt::FontRole)
        {
            QFont font;
            if(item.metadata.isMFS)
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


