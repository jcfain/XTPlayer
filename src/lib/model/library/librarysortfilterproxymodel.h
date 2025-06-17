#ifndef LIBRARYSORTFILTERPROXYMODEL_H
#define LIBRARYSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>
#include "lib/lookup/enum.h"
#include "lib/handler/medialibraryhandler.h"

class LibrarySortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public slots:
    void onFilterChanged(QString filter);
    void onTagFilterChanged(bool selected, QString filter);
public:
    explicit LibrarySortFilterProxyModel(MediaLibraryHandler* mediaLibraryHandler, QObject *parent = nullptr);
    void setSortMode(LibrarySortMode sortMode);
    void setLibraryViewMode(LibraryView mode);
    bool hasTags();
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    LibrarySortMode _sortMode = LibrarySortMode::NAME_ASC;
    bool dateInRange(QDate date) const;
    QString m_filterText;
    QStringList m_tags;

    QDate minDate;
    QDate maxDate;
};

#endif // LIBRARYSORTFILTERPROXYMODEL_H
