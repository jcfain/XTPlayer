#ifndef CHANNELMODEL_H
#define CHANNELMODEL_H

#include <QString>
#include <QMetaType>
#include <QDataStream>
#include "../lookup/AxisNames.h"


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
    int UserMin;
    int UserMid;
    int UserMax;
    AxisDimension Dimension;
    AxisType Type;
    QString TrackName;
    bool MultiplierEnabled;
    float MultiplierValue;
    bool DamperEnabled;
    float DamperValue;
    bool Inverted;

    friend QDataStream & operator<<( QDataStream &dataStream, const ChannelModel &object )
    {
        dataStream << object.FriendlyName;
        dataStream << object.AxisName;
        dataStream << object.Channel;
        dataStream << object.Min;
        dataStream << object.Mid;
        dataStream << object.Max;
        dataStream << object.UserMin;
        dataStream << object.UserMid;
        dataStream << object.UserMax;
        dataStream << object.Dimension;
        dataStream << object.Type;
        dataStream << object.TrackName;
        dataStream << object.MultiplierEnabled;
        dataStream << object.MultiplierValue;
        dataStream << object.DamperEnabled;
        dataStream << object.DamperValue;
        dataStream << object.Inverted;
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
        dataStream >> object.UserMin;
        dataStream >> object.UserMid;
        dataStream >> object.UserMax;
        dataStream >> object.Dimension;
        dataStream >> object.Type;
        dataStream >> object.TrackName;
        dataStream >> object.MultiplierEnabled;
        dataStream >> object.MultiplierValue;
        dataStream >> object.DamperEnabled;
        dataStream >> object.DamperValue;
        dataStream >> object.Inverted;
        return dataStream;
    }
};
Q_DECLARE_METATYPE(ChannelModel);
#endif // CHANNELMODEL_H
