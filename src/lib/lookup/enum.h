#ifndef ENUM_H
#define ENUM_H
#include <QObject>
enum DeviceType
{
    Serial,
    Network,
    Deo,
    Whirligig,
    Gamepad,
    XTPWeb
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
    TYPE_DESC,
    NONE
};

enum PasswordResponse {
    CORRECT,
    INCORRECT,
    CANCEL
};

enum TCodeVersion {
    v2,
    v3
};
Q_DECLARE_METATYPE(TCodeVersion);
#endif // ENUM_H
