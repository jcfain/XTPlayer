#include "librarysortfilterproxymodel.h"

#include "lib/handler/settingshandler.h"
#include "librarylistviewmodel.h"
#include "lib/struct/LibraryListItem27.h"
#include "xtpsettings.h"

LibrarySortFilterProxyModel::LibrarySortFilterProxyModel(MediaLibraryHandler* mediaLibraryHandler, QObject *parent)
    : QSortFilterProxyModel(parent) {
    setDynamicSortFilter(false);
    setSortRole(Qt::UserRole);
    connect(mediaLibraryHandler, &MediaLibraryHandler::libraryChange, this,  [this]() {
        invalidate();
    } );
    connect(mediaLibraryHandler, &MediaLibraryHandler::libraryLoaded, this,  [this]() {
        invalidate();
    } );
//    connect(mediaLibraryHandler, &MediaLibraryHandler::itemUpdated, this,   [this](LibraryListItem27 item) {
//        invalidate();
//    } );
    connect(mediaLibraryHandler, &MediaLibraryHandler::itemRemoved, this,   [this](int indexOfItem, int newSize) {
/*        auto index = this->index(indexOfItem, 0);
        emit dataChanged(index, index)*/;
        invalidate();
    } );
    connect(mediaLibraryHandler, &MediaLibraryHandler::itemAdded, this,   [this](int indexOfItem, int newSize) {
//        auto index = this->index(indexOfItem, 0);
//        emit dataChanged(index, index);

        invalidate();
    } );
}

void LibrarySortFilterProxyModel::setSortMode(LibrarySortMode sortMode) {
    if(_sortMode != sortMode || _sortMode == LibrarySortMode::RANDOM) {
        //beginResetModel();
        _sortMode = sortMode;
        invalidate();
        sort(0);
        //endResetModel();
    }
}

void LibrarySortFilterProxyModel::setLibraryViewMode(LibraryView mode) {
    beginResetModel();
    qobject_cast<LibraryListViewModel*>(sourceModel())->setLibraryViewMode(mode);
    endResetModel();
}

bool LibrarySortFilterProxyModel::hasTags()
{
    return !m_tags.empty();
}

bool LibrarySortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {

//    if(!left.isValid() || !right.isValid())
//        return false;
    LibraryListItem27 leftData = left.data(sortRole()).value<LibraryListItem27>();
    LibraryListItem27 rightData = right.data(sortRole()).value<LibraryListItem27>();

    if(_sortMode != NONE && left.column()== 0)
    {
        if(leftData.type == LibraryListItemType::PlaylistInternal && rightData.type != LibraryListItemType::PlaylistInternal)
        {
            return true;
        }
        else if(leftData.type != LibraryListItemType::PlaylistInternal && rightData.type == LibraryListItemType::PlaylistInternal)
        {
            return false;
        }
        else if(leftData.type == LibraryListItemType::PlaylistInternal && rightData.type == LibraryListItemType::PlaylistInternal &&
                (_sortMode == LibrarySortMode::NAME_DESC || _sortMode == LibrarySortMode::TYPE_DESC))
        {
            return leftData.nameNoExtension.localeAwareCompare(rightData.nameNoExtension) > 0;
        }
        else if(leftData.type == LibraryListItemType::PlaylistInternal && rightData.type == LibraryListItemType::PlaylistInternal)
        {
            return leftData.nameNoExtension.localeAwareCompare(rightData.nameNoExtension) < 0;
        }
        else if(_sortMode == LibrarySortMode::RANDOM)
        {
            return rand()%20 < 10;
        }
        else if(_sortMode == LibrarySortMode::CREATED_ASC)
        {
            return leftData.modifiedDate < rightData.modifiedDate;
        }
        else if(_sortMode == LibrarySortMode::CREATED_DESC)
        {
            return leftData.modifiedDate > rightData.modifiedDate;
        }
        else if(_sortMode == LibrarySortMode::NAME_ASC)
        {
            return left.data().toString().localeAwareCompare(right.data().toString()) < 0;
        }
        else if(_sortMode == LibrarySortMode::NAME_DESC)
        {
            return left.data().toString().localeAwareCompare(right.data().toString()) > 0;
        }
        else if(_sortMode == LibrarySortMode::TYPE_ASC)
        {
            if (leftData.type == rightData.type)
                return left.data().toString().localeAwareCompare(right.data().toString()) < 0;
            return leftData.type < rightData.type;
        }
        else if(_sortMode == LibrarySortMode::TYPE_DESC)
        {
            if (leftData.type == rightData.type)
                return left.data().toString().localeAwareCompare(right.data().toString()) > 0;
            return leftData.type > rightData.type;
        }
    }

    return QSortFilterProxyModel::lessThan(left, right);
}
#include <QSet>
bool LibrarySortFilterProxyModel::filterAcceptsRow(int sourceRow,
                                              const QModelIndex &sourceParent) const
{
    //Slow!?
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    LibraryListItem27 item = index.data(Qt::UserRole).value<LibraryListItem27>();
    bool isHidden = (item.type == LibraryListItemType::VR && !SettingsHandler::getShowVRInLibraryView()) ||
            ( item.type == LibraryListItemType::FunscriptType && SettingsHandler::getHideStandAloneFunscriptsInLibrary())  ||
            ( item.type != LibraryListItemType::FunscriptType &&
                item.type != LibraryListItemType::PlaylistInternal &&
                !item.hasScript &&
                XTPSettings::getHideMediaWithoutFunscripts()) ;
    if(isHidden)
        return !isHidden;


    //For some reason sorting random without any playlists crashes. Add dummy and hide it.
    if(item.nameNoExtension == "DummyPlaylistThatNoOneShouldEverSeeOrNameTheSame")
        return false;
    bool hasAllTags = m_tags.empty();
    foreach (QString tag, m_tags) {
        if(!item.metadata.tags.contains(tag)) {
            hasAllTags = false;
            break;
        }
        if(m_tags.indexOf(tag) == m_tags.length() -1)
            hasAllTags = true;
    }
    return index.data().toString().contains(filterRegularExpression()) && hasAllTags;
}

bool LibrarySortFilterProxyModel::dateInRange(QDate date) const
{
    return (!minDate.isValid() || date > minDate)
            && (!maxDate.isValid() || date < maxDate);
}

void LibrarySortFilterProxyModel::onFilterChanged(QString filter)
{
    m_filterText = filter;
    QRegularExpression::PatternOptions options = QRegularExpression::CaseInsensitiveOption;
    QRegularExpression regularExpression(filter, options);
    setFilterRegularExpression(regularExpression);
    invalidateFilter();
    if(filter.isEmpty())
        invalidate();
}

void LibrarySortFilterProxyModel::onTagFilterChanged(bool selected, QString filter)
{
    if(selected) {
        if(!m_tags.contains(filter))
            m_tags.append(filter);
    } else {
        m_tags.removeAll(filter);
    }
    invalidateFilter();
    if(m_tags.isEmpty())
        invalidate();
}
