#ifndef CHANNELTABLEVIEWMODEL_H
#define CHANNELTABLEVIEWMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include "ChannelModel.h"
#include "../handler/serialhandler.h"

class ChannelTableViewModel : public QAbstractTableModel
{
    Q_OBJECT
signals:
    void editCompleted(const QString &);
public:
    explicit ChannelTableViewModel(QObject *parent = 0);
    enum MapRoles {
        KeyRole = Qt::UserRole + 1,
        ValueRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    inline void setMap()
    {
        beginResetModel();
        _map = SettingsHandler::getAvailableAxis();
        endResetModel();
    }
    const ChannelModel* getRowData(int row);

private:
    QMap<QString, ChannelModel>* _map;
    AxisNames axisNames;
};

#endif // CHANNELTABLEVIEWMODEL_H
