#ifndef CHANNELMODEL_H
#define CHANNELMODEL_H

#include <QString>
#include <QMetaType>
#include <QDataStream>
#include <QVariant>
#include <QJsonObject>
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
    bool LinkToRelatedMFS;
    QString RelatedChannel;

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
        dataStream << object.LinkToRelatedMFS;
        dataStream << object.RelatedChannel;
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
        dataStream >> object.LinkToRelatedMFS;
        dataStream >> object.RelatedChannel;
        return dataStream;
    }

    static ChannelModel fromVariant(QVariant item)
    {
        QJsonObject obj = item.toJsonObject();
        ChannelModel newItem;
        newItem.FriendlyName = obj["friendlyName"].toString();
        newItem.AxisName = obj["axisName"].toString();
        newItem.Channel = obj["channel"].toString();
        newItem.Min = obj["min"].toInt();
        newItem.Mid = obj["mid"].toInt();
        newItem.Max = obj["max"].toInt();
        newItem.UserMin = obj["userMin"].toInt();
        newItem.UserMid = obj["userMid"].toInt();
        newItem.UserMax = obj["userMax"].toInt();
        newItem.Dimension = (AxisDimension)obj["dimension"].toInt();
        newItem.Type = (AxisType)obj["type"].toInt();
        newItem.TrackName = obj["trackName"].toString();
        newItem.MultiplierEnabled = obj["multiplierEnabled"].toBool();
        newItem.MultiplierValue = obj["multiplierValue"].toDouble();
        newItem.DamperEnabled = obj["damperEnabled"].toBool();
        newItem.DamperValue = obj["damperValue"].toDouble();
        newItem.Inverted = obj["inverted"].toBool();
        newItem.LinkToRelatedMFS = obj["linkToRelatedMFS"].toBool();
        newItem.RelatedChannel = obj["relatedChannel"].toString();
        return newItem;
    }

    static QVariant toVariant(ChannelModel item)
    {
        QJsonObject obj;
        obj["friendlyName"] = item.FriendlyName;
        obj["axisName"] = item.AxisName;
        obj["channel"] = item.Channel;
        obj["min"] = item.Min;
        obj["mid"] = item.Mid;
        obj["max"] = item.Max;
        obj["userMin"] = item.UserMin;
        obj["userMid"] = item.UserMid;
        obj["userMax"] = item.UserMax;
        obj["dimension"] = (int)item.Dimension;
        obj["type"] = (int)item.Type;
        obj["trackName"] = item.TrackName;
        obj["multiplierEnabled"] = item.MultiplierEnabled;
        obj["multiplierValue"] = item.MultiplierValue;
        obj["damperEnabled"] = item.DamperEnabled;
        obj["damperValue"] = item.DamperValue;
        obj["inverted"] = item.Inverted;
        obj["linkToRelatedMFS"] = item.LinkToRelatedMFS;
        obj["relatedChannel"] = item.RelatedChannel;
        return QVariant::fromValue(obj);
     }
};
Q_DECLARE_METATYPE(ChannelModel);
#endif // CHANNELMODEL_H
