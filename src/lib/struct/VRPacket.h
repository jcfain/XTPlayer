#ifndef VRPACKET_H
#define VRPACKET_H
#include <QString>
#include <QMetaType>

struct VRPacket
{
    QString path;
    qint64 duration;
    qint64 currentTime;
    float playbackSpeed;
    bool playing;
};

Q_DECLARE_METATYPE(VRPacket);
#endif // DEOPACKET_H
