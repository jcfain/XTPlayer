#include "librarysortfilterproxymodel.h"

LibrarySortFilterProxyModel::LibrarySortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{

}
void LibrarySortFilterProxyModel::setSortMode(LibrarySortMode sortMode) {
    _sortMode = sortMode;
}
bool LibrarySortFilterProxyModel::lessThan(const QModelIndex &left,
                                      const QModelIndex &right) const
{
    LibraryListItem27 leftData = sourceModel()->data(left).value<LibraryListItem27>();
    LibraryListItem27 rightData = sourceModel()->data(right).value<LibraryListItem27>();

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
        else if(leftData.type == LibraryListItemType::PlaylistInternal && rightData.type == LibraryListItemType::PlaylistInternal)
        {
            return rightData.nameNoExtension.localeAwareCompare(leftData.nameNoExtension) > 0;
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
        else if(_sortMode == LibrarySortMode::NAME_DESC)
        {
            return leftData.name.localeAwareCompare(rightData.name) > 0;
        }
        else if(_sortMode == LibrarySortMode::NAME_ASC)
        {

        }
        else if(_sortMode == LibrarySortMode::TYPE_ASC)
        {
            if (leftData.type == rightData.type)
              return rightData.name.localeAwareCompare(leftData.name) > 0;
            return leftData.type < rightData.type;
        }
        else if(_sortMode == LibrarySortMode::TYPE_DESC)
        {
            if (leftData.type == rightData.type)
              return leftData.name.localeAwareCompare(rightData.name) > 0;
            return leftData.type > rightData.type;
        }
    }
    // otherwise just return the comparison result from the base class
    return false;
}
bool LibrarySortFilterProxyModel::filterAcceptsRow(int sourceRow,
                                              const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

    return (sourceModel()->data(index0).toString().contains(filterRegularExpression()));
}
bool LibrarySortFilterProxyModel::dateInRange(QDate date) const
{
    return (!minDate.isValid() || date > minDate)
            && (!maxDate.isValid() || date < maxDate);
}
