#ifndef DECODERMODEL_H
#define DECODERMODEL_H

#include <QString>
#include <QMetaType>
#include <QDataStream>
#include <QJsonObject>

struct DecoderModel
{
    QString Name;
    bool Enabled;

    friend QDataStream & operator<<( QDataStream &dataStream, const DecoderModel &object )
    {
        dataStream << object.Name;
        dataStream << object.Enabled;
        return dataStream;
    }

    friend QDataStream & operator>>(QDataStream &dataStream, DecoderModel &object)
    {
        dataStream >> object.Name;
        dataStream >> object.Enabled;
        return dataStream;
    }

    static DecoderModel fromVariant(QVariant item)
    {
        QJsonObject obj = item.toJsonObject();
        DecoderModel newItem;
        newItem.Name = obj["name"].toString();
        newItem.Enabled = obj["enabled"].toBool();
        return newItem;
    }

    static QVariant toVariant(DecoderModel item)
    {
        QJsonObject obj;
        obj["name"] = item.Name;
        obj["enabled"] = item.Enabled;
        return QVariant::fromValue(obj);
    }
};
Q_DECLARE_METATYPE(DecoderModel);
#endif // DECODERMODEL_H
