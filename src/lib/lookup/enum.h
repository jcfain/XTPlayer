#ifndef ENUM_H
#define ENUM_H

enum DeviceType
{
    Serial,
    Network,
    Deo,
    Whirligig,
    Gamepad
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

enum LibrarySortMode {
    NAME_ASC,
    NAME_DESC,
    RANDOM,
    CREATED_ASC,
    CREATED_DESC,
    TYPE_ASC,
    TYPE_DESC
};

#endif // ENUM_H
