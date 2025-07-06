#include "channeltableviewmodel.h"
#include "lib/handler/settingshandler.h"

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
    return _map.count();
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
    if (_map.length() == 0)
        return QVariant();
    if (index.row() < 0 || index.row() >= _map.count()) {
        return QVariant();
    }
    switch (role) {
      case Qt::DisplayRole:
      {
        auto channel = TCodeChannelLookup::getChannel(_map[index.row()]);
        if(channel == nullptr)
            return QVariant();
//        if (index.column() == 0)
//            return _map->keys().at(index.row());
        if (index.column() == 0)
            return channel->FriendlyName;
        else if (index.column() == 1)
            return channel->ChannelName;
        else if (index.column() == 2)
            return channel->Channel;
        else if (index.column() == 3)
            return channel->Min;
        else if (index.column() == 4)
            return channel->Mid;
        else if (index.column() == 5)
            return channel->Max;
        else if (index.column() == 6)
            return (int)channel->Dimension;
        else if (index.column() == 7)
            return (int)channel->Type;
        else if (index.column() == 8)
            return channel->trackName;
      }
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

        QString key = _map[index.row()];
        auto channel = TCodeChannelLookup::getChannel(key);
        //save value from editor to member m_gridData
//        if (index.column() == 0)
//        {
//            return false;
//        }
        if (index.column() == 0)
        {
            channel->FriendlyName = value.toString();
            //SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 1)
        {
            channel->ChannelName = value.toString();
            //SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 2)
        {
            channel->Channel = value.toString();
            //SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 3)
        {
            channel->Min = value.toInt();
            //SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 4)
        {
            channel->Mid = value.toInt();
            //SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 5)
        {
            channel->Max = value.toInt();
            //SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 6)
        {
            channel->Dimension = (ChannelDimension)value.toInt();
            //SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 7)
        {
            channel->Type = (ChannelType)value.toInt();
            //SettingsHandler::setAxis(key, valueModel);
        }
        else if (index.column() == 8)
        {
            channel->trackName = value.toString();
            //SettingsHandler::setAxis(key, valueModel);
        }
        //setMap(_map);
        emit editCompleted("");
        return true;
    }
    return false;
}

Qt::ItemFlags ChannelTableViewModel::flags(const QModelIndex &index) const
{
    if (index.column() == 1 || index.column() == 2)
        return Qt::ItemIsSelectable | QAbstractTableModel::flags(index);
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

const QString ChannelTableViewModel::getRowKey(int row)
{
    if (row < 0)
        return nullptr;
    return _map[row];
}
