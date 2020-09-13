#ifndef DEOPACKET_H
#define DEOPACKET_H
#include <QString>
#include <QMetaType>

struct DeoPacket
{
    QString path;
    qint64 duration;
    qint64 currentTime;
    float playbackSpeed;
    bool playing;
};

Q_DECLARE_METATYPE(DeoPacket);
#endif // DEOPACKET_H
