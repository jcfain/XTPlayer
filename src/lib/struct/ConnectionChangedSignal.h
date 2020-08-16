#ifndef CONNECTIONCHANGEDSIGNAL_H
#define CONNECTIONCHANGEDSIGNAL_H
#include <QString>
#include "../lookup/enum.h"

struct ConnectionChangedSignal
{
    DeviceType deviceType;
    ConnectionStatus status;
    QString message;
};

#endif // CONNECTIONCHANGEDSIGNAL_H
