#ifndef ENUM_H
#define ENUM_H

enum DeviceType
{
    Serial,
    Network
};

enum ConnectionStatus
{
    Connected,
    Disconnected,
    Connecting,
    Error
};

enum LibraryView
{
    Thumb,
    List
};

#endif // ENUM_H
