#include "librarylistviewmodel.h"

#include "lib/tool/imagefactory.h"
#include "lib/handler/settingshandler.h"

LibraryListViewModel::LibraryListViewModel(MediaLibraryHandler* mediaLibraryHandler, QObject *parent)
    : QAbstractListModel(parent)
{
    _mediaLibraryHandler = mediaLibraryHandler;
    connect(_mediaLibraryHandler, &MediaLibraryHandler::libraryLoaded, this,  [this]() {
        beginResetModel();
        m_librarySize = getData()->length();
        QModelIndex topLeft = createIndex(0,0);
        QModelIndex bottomRight = createIndex( m_librarySize, 0);
        emit dataChanged( topLeft, bottomRight );
        endResetModel();
    } );
    connect(_mediaLibraryHandler, &MediaLibraryHandler::metadataProcessEnd, this,  [this]() {
        beginResetModel();
        m_librarySize = getData()->length();
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

int LibraryListViewModel::getThumbInt() const
{
    return overRideThumbSizeWidth > -1 ? overRideThumbSizeWidth : SettingsHandler::getThumbSize();
}

const QList<LibraryListItem27>* LibraryListViewModel::getData() const {
    return _mediaLibraryHandler->getLibraryCache()->getItems();
}
QVariant LibraryListViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= m_librarySize)
        return QVariant();

    if (index.column() == 0) {
        // LogHandler::Debug("[LibraryListViewModel::data]");
        _mediaLibraryHandler->getLibraryCache()->lockForRead();
        auto data =  getData();

        const LibraryListItem27 item = data->value(index.row());
        if (role == Qt::DisplayRole)
        {
            QString displayText = (item.metadata.isSFMA && item.metadata.isMFS ? "(SFMA) (MFS) " : item.metadata.isSFMA ? "(SFMA) " : item.metadata.isMFS ? "(MFS) " : "") + item.nameNoExtension;
            _mediaLibraryHandler->getLibraryCache()->unlock();
            return displayText;
        }
        else if(Qt::DecorationRole == role)
        {
            int scaled = qRound(getThumbInt() * 0.75);
            QSize thumbSize = {scaled, scaled};
            if(item.thumbState == ThumbState::Waiting) {
                _mediaLibraryHandler->getLibraryCache()->unlock();
                return ImageFactory::resizeCache(LOADING_IMAGE, item.ID, thumbSize);
            }
            else if(item.thumbState == ThumbState::Loading) {
                _mediaLibraryHandler->getLibraryCache()->unlock();
                return ImageFactory::resizeCache(LOADING_CURRENT_IMAGE, item.ID, thumbSize);
            }
            else if(item.thumbState == ThumbState::Error) {
                _mediaLibraryHandler->getLibraryCache()->unlock();
                return ImageFactory::resizeCache(ERROR_IMAGE, item.ID, thumbSize);
            }
            else if(item.thumbState == ThumbState::Unknown) {
                _mediaLibraryHandler->getLibraryCache()->unlock();
                return ImageFactory::resizeCache(UNKNOWN_IMAGE, item.ID, thumbSize);
            }
            _mediaLibraryHandler->getLibraryCache()->unlock();
            return ImageFactory::resizeCache(item.thumbFile, item.ID, thumbSize);
        }
        else if(role == Qt::UserRole)
        {
            auto variant = QVariant::fromValue(item);
            _mediaLibraryHandler->getLibraryCache()->unlock();
            return variant;
        }
        else if (role == Qt::ToolTipRole)
        {
            QString tooltip = item.metadata.toolTip;
            if(!item.metadata.thumbExtractError.isEmpty()) {
                tooltip +=  "\n" +item.metadata.thumbExtractError;
            }
            _mediaLibraryHandler->getLibraryCache()->unlock();
            return tooltip;
        }
        else if (role == Qt::ForegroundRole)
        {
            if(item.type != LibraryListItemType::PlaylistInternal)
            {
                if(item.error) {
                    _mediaLibraryHandler->getLibraryCache()->unlock();
                    return QColor(Qt::GlobalColor::red);
                }
                if (!item.hasScript && item.metadata.isMFS) {
                    _mediaLibraryHandler->getLibraryCache()->unlock();
                    return QColor(Qt::GlobalColor::cyan);
                }
                if (!item.hasScript) {
                    _mediaLibraryHandler->getLibraryCache()->unlock();
                    return QColor(Qt::gray);
                }
                if(item.metadata.isSFMA) {
                    _mediaLibraryHandler->getLibraryCache()->unlock();
                    return QColor(Qt::GlobalColor::yellow);
                }
                if(item.metadata.isMFS) {
                    _mediaLibraryHandler->getLibraryCache()->unlock();
                    return QColor(Qt::green);
                }
            }
        }
        else if (role == Qt::BackgroundRole)
        {
            _mediaLibraryHandler->getLibraryCache()->unlock();
            return QColor(Qt::transparent);
        }
        else if (role == Qt::FontRole)
        {
            QFont font;
            if(item.metadata.isMFS || item.metadata.isSFMA)
                font.setBold(true);
            if(_libraryViewMode == LibraryView::Thumb)
                font.setPointSizeF((getThumbInt() * 0.25) * 0.25);
            else
                font.setPointSizeF((getThumbInt() * 0.25) * 0.35);

            _mediaLibraryHandler->getLibraryCache()->unlock();
            return font;
        }
        else if (role == Qt::SizeHintRole)
        {
            if(_libraryViewMode == LibraryView::Thumb) {
                auto thumbInt = getThumbInt();
                int scaled  = qRound(thumbInt * 0.25) +thumbInt;
                QSize thumbSizeHint = {scaled, scaled};
                _mediaLibraryHandler->getLibraryCache()->unlock();
                return thumbSizeHint;
            }
        }
        else if (role == Qt::TextAlignmentRole)
        {
            _mediaLibraryHandler->getLibraryCache()->unlock();
            return _libraryViewMode == LibraryView::Thumb ?
                        int(Qt::AlignmentFlag::AlignTop | Qt::AlignmentFlag::AlignHCenter) :
                        int(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);
        }
        _mediaLibraryHandler->getLibraryCache()->unlock();
    }


    return QVariant();
}


