#ifndef DECODERMODEL_H
#define DECODERMODEL_H

#include <QString>
#include <QMetaType>
#include <QDataStream>

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
};
Q_DECLARE_METATYPE(DecoderModel);
#endif // DECODERMODEL_H
