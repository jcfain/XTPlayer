#include "librarysortfilterproxymodel.h"

LibrarySortFilterProxyModel::LibrarySortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{

}
void LibrarySortFilterProxyModel::setSortMode(LibrarySortMode sortMode) {
    if(_sortMode != sortMode) {
        beginResetModel();
        _sortMode = sortMode;
        sort(0);
        invalidate();
        endResetModel();
    }
}
bool LibrarySortFilterProxyModel::lessThan(const QModelIndex &left,
                                      const QModelIndex &right) const
{
    LibraryListItem27 leftData = left.data(sortRole()).value<LibraryListItem27>();
    LibraryListItem27 rightData = right.data(sortRole()).value<LibraryListItem27>();

    if(_sortMode != NONE)
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
    //            qint64 randomValue = XMath::rand(0, 100);
    //            if(randomValue > 50)
    //                return thisData.modifiedDate < otherData.modifiedDate;
            return false;
        }
        else if(_sortMode == LibrarySortMode::CREATED_ASC)
        {
            return leftData.modifiedDate < rightData.modifiedDate;
    //            auto d1 = thisData.modifiedDate;
    //            auto d2 = otherData.modifiedDate;
    //            if (d1.year() < d2.year())
    //                return true;
    //            if (d1.year() == d2.year() && d1.month() < d2.month())
    //                return true;
    //            if (d1.year() == d2.year() && d1.month() == d2.month() && d1.day() < d2.day())
    //                return true;

    //            return false;
        }
        else if(_sortMode == LibrarySortMode::CREATED_DESC)
        {
            return leftData.modifiedDate > rightData.modifiedDate;
    //            auto d1 = thisData.modifiedDate;
    //            auto d2 = otherData.modifiedDate;
    //            if (d1.year() > d2.year())
    //                return true;
    //            if (d1.year() == d2.year() && d1.month() > d2.month())
    //                return true;
    //            if (d1.year() == d2.year() && d1.month() == d2.month() && d1.day() > d2.day())
    //                return true;

    //            return false;
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
    // otherwise just return the comparison result from the base class
    return false;
}
bool LibrarySortFilterProxyModel::filterAcceptsRow(int sourceRow,
                                              const QModelIndex &sourceParent) const
{
    //Slow!?
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    LibraryListItem27 item = index.data(Qt::UserRole).value<LibraryListItem27>();
    bool isHidden = item.type == LibraryListItemType::VR || (SettingsHandler::getHideStandAloneFunscriptsInLibrary() && item.type == LibraryListItemType::FunscriptType);
    if(isHidden)
        return !isHidden;
//    return (sourceModel()->data(index0, Qt::UserRole).value<LibraryListItem27>().name.contains(filterRegularExpression()));
    return true;
}
bool LibrarySortFilterProxyModel::dateInRange(QDate date) const
{
    return (!minDate.isValid() || date > minDate)
            && (!maxDate.isValid() || date < maxDate);
}
