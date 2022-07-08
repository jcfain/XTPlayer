#include "channeltableviewmodel.h"

ChannelTableViewModel::ChannelTableViewModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    setMap();
    //setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    setHeaderData(0, Qt::Horizontal, QObject::tr("Friendly Name"));
    setHeaderData(1, Qt::Horizontal, QObject::tr("Axis Name"));
    setHeaderData(2, Qt::Horizontal, QObject::tr("Channel"));
    setHeaderData(3, Qt::Horizontal, QObject::tr("Min"));
    setHeaderData(4, Qt::Horizontal, QObject::tr("Mid"));
    setHeaderData(5, Qt::Horizontal, QObject::tr("Max"));
    setHeaderData(6, Qt::Horizontal, QObject::tr("Dimension"));
    setHeaderData(7, Qt::Horizontal, QObject::tr("Type"));
    setHeaderData(8, Qt::Horizontal, QObject::tr("Track"));

}

int ChannelTableViewModel::rowCount(const QModelIndex& parent) const
{
    if (_map)
        return _map->count();
    return 0;
}

int ChannelTableViewModel::columnCount(const QModelIndex & parent) const
{
    return 9;
}

QVariant ChannelTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
//        case 0:
//            return QString("ID");
        case 0:
            return QString("Friendly Name");
        case 1:
            return QString("Axis Name");
        case 2:
            return QString("Channel");
        case 3:
            return QString("Min");
        case 4:
            return QString("Mid");
        case 5:
            return QString("Max");
        case 6:
            return QString("Dimension");
        case 7:
            return QString("Type");
        case 8:
            return QString("Track");
        }
    }
    return QVariant();
}

QVariant ChannelTableViewModel::data(const QModelIndex& index, int role) const
{
    if (!_map)
        return QVariant();
    if (index.row() < 0 || index.row() >= _map->count()) {
        return QVariant();
    }
    switch (role) {
      case Qt::DisplayRole:
//        if (index.column() == 0)
//            return _map->keys().at(index.row());
        if (index.column() == 0)
            return (_map->constBegin() + index.row()).value().FriendlyName;
        else if (index.column() == 1)
            return (_map->constBegin() + index.row()).value().AxisName;
        else if (index.column() == 2)
            return (_map->constBegin() + index.row()).value().Channel;
        else if (index.column() == 3)
            return (_map->constBegin() + index.row()).value().Min;
        else if (index.column() == 4)
            return (_map->constBegin() + index.row()).value().Mid;
        else if (index.column() == 5)
            return (_map->constBegin() + index.row()).value().Max;
        else if (index.column() == 6)
            return (int)(_map->constBegin() + index.row()).value().Dimension;
        else if (index.column() == 7)
            return (int)(_map->constBegin() + index.row()).value().Type;
        else if (index.column() == 8)
            return (_map->constBegin() + index.row()).value().TrackName;
      case Qt::FontRole:
          break;
      case Qt::BackgroundRole:
          break;
      case Qt::TextAlignmentRole:
          break;
      case Qt::CheckStateRole:
          break;
      }
    return QVariant();
}

bool ChannelTableViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (!checkIndex(index))
            return false;
        if (value.isNull())
            return false;
        if ((value.userType() == QMetaType::QString && value.toString().isEmpty()))
            return false;
        //save value from editor to member m_gridData
        QString key = (_map->constBegin() + index.row()).key();
        ChannelModel valueModel = (_map->constBegin() + index.row()).value();
//        if (index.column() == 0)
//        {
//            return false;
//        }
        if (index.column() == 0)
        {
            valueModel.FriendlyName = value.toString();
            SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 1)
        {
            valueModel.AxisName = value.toString();
            SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 2)
        {
            valueModel.Channel = value.toString();
            SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 3)
        {
            valueModel.Min = value.toInt();
            SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 4)
        {
            valueModel.Mid = value.toInt();
            SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 5)
        {
            valueModel.Max = value.toInt();
            SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 6)
        {
            valueModel.Dimension = (AxisDimension)value.toInt();
            SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 7)
        {
            valueModel.Type = (AxisType)value.toInt();
            SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 9)
        {
            valueModel.TrackName = value.toString();
            SettingsHandler::setAxis(key, valueModel);
        }
        setMap();
        emit editCompleted("");
        return true;
    }
    return false;
}

Qt::ItemFlags ChannelTableViewModel::flags(const QModelIndex &index) const
{
    QString key = (_map->constBegin() + index.row()).key();
    if (index.column() == 1 || index.column() == 2)
        return Qt::ItemIsSelectable | QAbstractTableModel::flags(index);
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

const ChannelModel* ChannelTableViewModel::getRowData(int row)
{
    if (!_map || row <= 0)
        return nullptr;
    return &(_map->constBegin() + row).value();
}
