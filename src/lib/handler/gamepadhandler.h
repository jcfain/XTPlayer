#ifndef GAMEPADHANDLER_H
#define GAMEPADHANDLER_H

#include <QGamepad>
#include "devicehandler.h"
#include "../tool/tcodefactory.h"
#include "../lookup/MediaActions.h"


class GamepadHandler : public DeviceHandler
{
public:
    GamepadHandler(QObject *parent = nullptr);
    ~GamepadHandler();
    QHash<QString, QVariant>* getState();
    void init();
    void dispose() override;
    bool isConnected() override;


private:
    void run() override;
    void gamePadConnectionChanged(bool connected);
    void connectedGamepadsChanged();
    void disposeInternal();
    double calculateDeadZone(double gpIn);
    QList<int> _gamepads;
    QGamepad* _gamepad = nullptr;
    QHash<QString, QVariant>* _gamepadState = nullptr;
    QMutex _mutex;
    QWaitCondition _cond;
    int _waitTimeout = 0;
    bool _stop = false;
    bool _isConnected = false;
    double _deadzone = 0.2;
};

#endif // GAMEPADHANDLER_H
