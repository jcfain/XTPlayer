#ifndef CHANNELMODEL_H
#define CHANNELMODEL_H

#include <QString>
#include <QMetaType>
#include <QDataStream>


struct ChannelValueModel
{
    int Value;
    QString Channel;
    friend bool operator==(const ChannelValueModel &p1, const ChannelValueModel &p2)
    {
       if(p1.Channel == p2.Channel)
         return true;
       else
         return false;
    }
};

struct ChannelModel
{
    QString FriendlyName;
    QString AxisName;
    QString Channel;
    int Min;
    int Mid;
    int Max;

    friend QDataStream & operator<<( QDataStream &dataStream, const ChannelModel &object )
    {
        dataStream << object.FriendlyName;
        dataStream << object.AxisName;
        dataStream << object.Channel;
        dataStream << object.Min;
        dataStream << object.Mid;
        dataStream << object.Max;
        return dataStream;
    }

    friend QDataStream & operator>>(QDataStream &dataStream, ChannelModel &object)
    {
        dataStream >> object.FriendlyName;
        dataStream >> object.AxisName;
        dataStream >> object.Channel;
        dataStream >> object.Min;
        dataStream >> object.Mid;
        dataStream >> object.Max;
        return dataStream;
    }
};
Q_DECLARE_METATYPE(ChannelModel);
#endif // CHANNELMODEL_H
