#ifndef CHANNELMODEL_H
#define CHANNELMODEL_H

#include <QString>
#include <QMetaType>

struct ChannelModel
{
    QString FriendlyName;
    QString AxisName;
    QString Channel;
    int Min;
    int Mid;
    int Max;
};
Q_DECLARE_METATYPE(ChannelModel);
#endif // CHANNELMODEL_H
