#ifndef LIBRARYSORTFILTERPROXYMODEL_H
#define LIBRARYSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>
#include "lib/handler/settingshandler.h"
#include "lib/tool/xmath.h"
#include "lib/struct/LibraryListItem27.h"

#include "librarylistviewmodel.h"
#include "xtpsettings.h"

class LibrarySortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public slots:
    void onTextFilterChanged(QString filter);
public:
    explicit LibrarySortFilterProxyModel(MediaLibraryHandler* mediaLibraryHandler, QObject *parent = nullptr);
    void setSortMode(LibrarySortMode sortMode);
    void setLibraryViewMode(LibraryView mode);
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    LibrarySortMode _sortMode;
    bool dateInRange(QDate date) const;

    QDate minDate;
    QDate maxDate;
};

#endif // LIBRARYSORTFILTERPROXYMODEL_H
