#include "gamepadhandler.h"

GamepadHandler::GamepadHandler(QObject *parent):
    DeviceHandler(parent)
{
    qRegisterMetaType<ConnectionChangedSignal>();
}

GamepadHandler::~GamepadHandler()
{
    if (_gamepadState != nullptr)
        delete _gamepadState;
    if (_gamepad != nullptr)
        delete _gamepad;
}

void GamepadHandler::init()
{
    _stop = false;
    emit connectionChange({DeviceType::Gamepad, ConnectionStatus::Connecting, "Connecting..."});
    connect(QGamepadManager::instance(), &QGamepadManager::connectedGamepadsChanged, this, &GamepadHandler::connectedGamepadsChanged);
    connectedGamepadsChanged();
}

void GamepadHandler::connectedGamepadsChanged()
{
    _gamepads = QGamepadManager::instance()->connectedGamepads();
    if(_gamepads.count() > 0 && (_gamepad == nullptr || !_isConnected))
    {
        LogHandler::Debug("Gamepad connected: "+ QString::number(*_gamepads.begin()));
        LogHandler::Debug("Gamepads connected count: "+ QString::number(_gamepads.count()));
        _stop = false;
        _isConnected = true;
        _gamepad = new QGamepad(*_gamepads.begin(), this);
        emit connectionChange({DeviceType::Gamepad, ConnectionStatus::Connected, "Connected"});
        connect(_gamepad, &QGamepad::connectedChanged, this, &GamepadHandler::gamePadConnectionChanged);
        if(!isRunning())
            start();
    }
    else if (_gamepad != nullptr && !_gamepad->isConnected())
    {
        LogHandler::Debug("Gamepads disconnected: "+ QString::number(_gamepads.count()));
        _isConnected = false;
        disposeInternal();
    }
    else
    {
        LogHandler::Debug("Gamepad connectionchange count: "+ QString::number(_gamepads.count()));
    }
}

void GamepadHandler::gamePadConnectionChanged(bool connected)
{
    //_isConnected = connected;
    if(connected)
    {
        LogHandler::Debug("Gamepads connected event");
//        _stop = false;
//        emit connectionChange({DeviceType::Gamepad, ConnectionStatus::Connected, "Connected"});
//        start();
    }
    else
    {
        LogHandler::Debug("Gamepad disconnected event");
    }
}
void GamepadHandler::run()
{
    QString lastTCode;
    TCodeFactory* tcodeFactory = new TCodeFactory(0.0, 1.0);
    QVector<ChannelValueModel> axisValues;
    GamepadAxisNames gamepadAxisNames;
    AxisNames axisNames;
    MediaActions mediaActions;
    XTimer leftXAxisTimer;
    XTimer leftYAxisTimer;
    XTimer rightXAxisTimer;
    XTimer rightYAxisTimer;
    XTimer rightTriggerTimer;
    XTimer leftTriggerTimer;
    XTimer aTimer;
    XTimer bTimer;
    XTimer xTimer;
    XTimer yTimer;
    XTimer rightBumperTimer;
    XTimer leftBumperTimer;
    XTimer startTimer;
    XTimer selectTimer;
    XTimer dPadUpTimer;
    XTimer dPadDownTimer;
    XTimer dPadRightTimer;
    XTimer dPadLeftTimer;
    XTimer rightAxisButtonTimer;
    XTimer leftAxisButtonTimer;
    XTimer centerTimer;
    XTimer guideTimer;

    while(!_stop)
    {
        axisValues.clear();
        QString LeftXAxis = SettingsHandler::getGamePadMapButton(gamepadAxisNames.LeftXAxis);
        if (LeftXAxis != axisNames.None)
        {
            if (!mediaActions.Values.contains(LeftXAxis))
                tcodeFactory->calculate(LeftXAxis, calculateDeadZone(_gamepad->axisLeftX()), axisValues);
            else if(leftXAxisTimer.remainingTime() <= 0 && _gamepad->axisLeftX() != 0)
            {
                emit emitAction(LeftXAxis);
                leftXAxisTimer.init(500);
            }
        }

        QString LeftYAxis = SettingsHandler::getGamePadMapButton(gamepadAxisNames.LeftYAxis);
        if (LeftYAxis != axisNames.None)
        {
            if (!mediaActions.Values.contains(LeftYAxis))
                tcodeFactory->calculate(LeftYAxis, calculateDeadZone(-_gamepad->axisLeftY()), axisValues);
            else if(leftYAxisTimer.remainingTime() <= 0  && _gamepad->axisLeftY() != 0)
            {
                emit emitAction(LeftYAxis);
                leftYAxisTimer.init(500);
            }
        }

        QString RightXAxis = SettingsHandler::getGamePadMapButton(gamepadAxisNames.RightXAxis);
        if (RightXAxis != axisNames.None)
        {
            if (!mediaActions.Values.contains(RightXAxis))
                tcodeFactory->calculate(RightXAxis, calculateDeadZone(_gamepad->axisRightX()), axisValues);
            else if(rightXAxisTimer.remainingTime() <= 0 && _gamepad->axisRightX() != 0)
            {
                emit emitAction(RightXAxis);
                rightXAxisTimer.init(500);
            }
        }

        QString RightYAxis = SettingsHandler::getGamePadMapButton(gamepadAxisNames.RightYAxis);
        if (RightYAxis != axisNames.None)
        {
            if (!mediaActions.Values.contains(RightYAxis))
                tcodeFactory->calculate(RightYAxis, calculateDeadZone(-_gamepad->axisRightY()), axisValues);
            else if(rightYAxisTimer.remainingTime() <= 0 && _gamepad->axisRightY() != 0)
            {
                emit emitAction(RightYAxis);
                rightYAxisTimer.init(500);
            }
        }

        QString RightTrigger = SettingsHandler::getGamePadMapButton(gamepadAxisNames.RightTrigger);
        if (RightTrigger != axisNames.None)
        {
            if (!mediaActions.Values.contains(RightTrigger))
                tcodeFactory->calculate(RightTrigger, _gamepad->buttonR2(), axisValues);
            else if(rightTriggerTimer.remainingTime() <= 0 && _gamepad->buttonR2() != 0)
            {
                emit emitAction(RightTrigger);
                rightTriggerTimer.init(500);
            }
        }

        QString LeftTrigger = SettingsHandler::getGamePadMapButton(gamepadAxisNames.LeftTrigger);
        if (LeftTrigger != axisNames.None)
        {
            if (!mediaActions.Values.contains(LeftTrigger))
                tcodeFactory->calculate(LeftTrigger, _gamepad->buttonL2(), axisValues);
            else if(leftTriggerTimer.remainingTime() <= 0 && _gamepad->buttonL2() != 0)
            {
                emit emitAction(LeftTrigger);
                leftTriggerTimer.init(500);
            }
        }

        // Binary inputs
        QString A = SettingsHandler::getGamePadMapButton(gamepadAxisNames.A);
        if (A != axisNames.None)
        {
            if (!mediaActions.Values.contains(A))
                tcodeFactory->calculate(A, _gamepad->buttonA(), axisValues);
            else if(aTimer.remainingTime() <= 0 && _gamepad->buttonA())
            {
                emit emitAction(A);
                aTimer.init(500);
            }
        }

        QString B = SettingsHandler::getGamePadMapButton(gamepadAxisNames.B);
        if (B != axisNames.None)
        {
            if (!mediaActions.Values.contains(B))
                tcodeFactory->calculate(B, _gamepad->buttonB(), axisValues);
            else if(bTimer.remainingTime() <= 0 && _gamepad->buttonB())
            {
                emit emitAction(B);
                bTimer.init(500);
            }
        }

        QString X = SettingsHandler::getGamePadMapButton(gamepadAxisNames.X);
        if (X != axisNames.None)
        {
            if (!mediaActions.Values.contains(X))
                tcodeFactory->calculate(X, _gamepad->buttonX(), axisValues);
            else if(xTimer.remainingTime() <= 0 && _gamepad->buttonX())
            {
                emit emitAction(X);
                xTimer.init(500);
            }
        }

        QString Y = SettingsHandler::getGamePadMapButton(gamepadAxisNames.Y);
        if (Y != axisNames.None)
        {
            if (!mediaActions.Values.contains(Y))
                tcodeFactory->calculate(Y, _gamepad->buttonY(), axisValues);
            else if(yTimer.remainingTime() <= 0 && _gamepad->buttonY())
            {
                emit emitAction(Y);
                yTimer.init(500);
            }
        }

        QString RightBumper = SettingsHandler::getGamePadMapButton(gamepadAxisNames.RightBumper);
        if (RightBumper != axisNames.None)
        {
            if (!mediaActions.Values.contains(RightBumper))
                tcodeFactory->calculate(RightBumper, _gamepad->buttonR1(), axisValues);
            else if(rightBumperTimer.remainingTime() <= 0 && _gamepad->buttonR1())
            {
                emit emitAction(RightBumper);
                rightBumperTimer.init(500);
            }
        }

        QString LeftBumper = SettingsHandler::getGamePadMapButton(gamepadAxisNames.LeftBumper);
        if (LeftBumper != axisNames.None)
        {
            if (!mediaActions.Values.contains(LeftBumper))
                tcodeFactory->calculate(LeftBumper, _gamepad->buttonL1(), axisValues);
            else if(leftBumperTimer.remainingTime() <= 0 && _gamepad->buttonL1())
            {
                emit emitAction(LeftBumper);
                leftBumperTimer.init(500);
            }
        }

        QString Start = SettingsHandler::getGamePadMapButton(gamepadAxisNames.Start);
        if (Start != axisNames.None)
        {
            if (!mediaActions.Values.contains(Start))
                tcodeFactory->calculate(Start, _gamepad->buttonStart(), axisValues);
            else if(startTimer.remainingTime() <= 0 && _gamepad->buttonStart())
            {
                emit emitAction(Start);
                startTimer.init(500);
            }
        }
        QString Select = SettingsHandler::getGamePadMapButton(gamepadAxisNames.Select);
        if (Select != axisNames.None)
        {
            if (!mediaActions.Values.contains(Select))
                tcodeFactory->calculate(Select, _gamepad->buttonSelect(), axisValues);
            else if(selectTimer.remainingTime() <= 0 && _gamepad->buttonSelect())
            {
                emit emitAction(Select);
                selectTimer.init(500);
            }
        }

        QString DPadUp = SettingsHandler::getGamePadMapButton(gamepadAxisNames.DPadUp);
        if (DPadUp != axisNames.None)
        {
            if (!mediaActions.Values.contains(DPadUp))
                tcodeFactory->calculate(DPadUp, _gamepad->buttonUp(), axisValues);
            else if(dPadUpTimer.remainingTime() <= 0 && _gamepad->buttonUp())
            {
                emit emitAction(DPadUp);
                dPadUpTimer.init(500);
            }
        }
        QString DPadDown = SettingsHandler::getGamePadMapButton(gamepadAxisNames.DPadDown);
        if (DPadDown != axisNames.None)
        {
            if (!mediaActions.Values.contains(DPadDown))
                tcodeFactory->calculate(DPadDown, _gamepad->buttonDown(), axisValues);
            else if(dPadDownTimer.remainingTime() <= 0 && _gamepad->buttonDown())
            {
                emit emitAction(DPadDown);
                dPadDownTimer.init(500);
            }
        }

        QString DPadLeft = SettingsHandler::getGamePadMapButton(gamepadAxisNames.DPadLeft);
        if (DPadLeft != axisNames.None)
        {
            if (!mediaActions.Values.contains(DPadLeft))
                tcodeFactory->calculate(DPadLeft, _gamepad->buttonLeft(), axisValues);
            else if(dPadLeftTimer.remainingTime() <= 0 && _gamepad->buttonLeft())
            {
                emit emitAction(DPadLeft);
                dPadLeftTimer.init(500);
            }
        }

        QString DPadRight = SettingsHandler::getGamePadMapButton(gamepadAxisNames.DPadRight);
        if (DPadRight != axisNames.None)
        {
            if (!mediaActions.Values.contains(DPadRight))
                tcodeFactory->calculate(DPadRight, _gamepad->buttonRight(), axisValues);
            else if(dPadRightTimer.remainingTime() <= 0 && _gamepad->buttonRight())
            {
                emit emitAction(DPadRight);
                dPadRightTimer.init(500);
            }
        }

        QString RightAxisButton = SettingsHandler::getGamePadMapButton(gamepadAxisNames.RightAxisButton);
        if (RightAxisButton != axisNames.None)
        {
            if (!mediaActions.Values.contains(RightAxisButton))
                tcodeFactory->calculate(RightAxisButton, _gamepad->buttonR3(), axisValues);
            else if(rightAxisButtonTimer.remainingTime() <= 0 && _gamepad->buttonR3())
            {
                emit emitAction(RightAxisButton);
                rightAxisButtonTimer.init(500);
            }
        }

        QString LeftAxisButton = SettingsHandler::getGamePadMapButton(gamepadAxisNames.LeftAxisButton);
        if (LeftAxisButton != axisNames.None)
        {
            if (!mediaActions.Values.contains(LeftAxisButton))
                tcodeFactory->calculate(LeftAxisButton, _gamepad->buttonL3(), axisValues);
            else if(leftAxisButtonTimer.remainingTime() <= 0 && _gamepad->buttonL3())
            {
                emit emitAction(LeftAxisButton);
                leftAxisButtonTimer.init(500);
            }
        }

        QString Center = SettingsHandler::getGamePadMapButton(gamepadAxisNames.Center);
        if (Center != axisNames.None)
        {
            if (!mediaActions.Values.contains(Center))
                tcodeFactory->calculate(Center, _gamepad->buttonCenter(), axisValues);
            else if(centerTimer.remainingTime() <= 0 && _gamepad->buttonCenter())
            {
                emit emitAction(Center);
                centerTimer.init(500);
            }
        }

        QString Guide = SettingsHandler::getGamePadMapButton(gamepadAxisNames.Guide);
        if (Guide != axisNames.None)
        {
            if (!mediaActions.Values.contains(Guide))
                tcodeFactory->calculate(Guide, _gamepad->buttonGuide(), axisValues);
            else if(guideTimer.remainingTime() <= 0 && _gamepad->buttonGuide())
            {
                emit emitAction(Guide);
                guideTimer.init(500);
            }
        }

        QString currentTCode = tcodeFactory->formatTCode(&axisValues);
        if (lastTCode != currentTCode)
        {
            lastTCode = currentTCode;
            emit emitTCode(currentTCode);
        }
        msleep(1);
    }
    leftXAxisTimer.stop();
    leftYAxisTimer.stop();
    rightXAxisTimer.stop();
    rightYAxisTimer.stop();
    rightTriggerTimer.stop();
    leftTriggerTimer.stop();
    aTimer.stop();
    bTimer.stop();
    xTimer.stop();
    yTimer.stop();
    rightBumperTimer.stop();
    leftBumperTimer.stop();
    startTimer.stop();
    selectTimer.stop();
    dPadUpTimer.stop();
    dPadDownTimer.stop();
    dPadRightTimer.stop();
    dPadLeftTimer.stop();
    rightAxisButtonTimer.stop();
    leftAxisButtonTimer.stop();
    centerTimer.stop();
    guideTimer.stop();
    delete tcodeFactory;
}

double GamepadHandler::calculateDeadZone(double gpIn)
{
    if ((gpIn < _deadzone && gpIn > 0) || (gpIn > -_deadzone && gpIn < 0))
    {
        return 0;
    }
    return gpIn;
}

void GamepadHandler::disposeInternal()
{
    _mutex.lock();
    _stop = true;
    _isConnected = false;
    if (_gamepad != nullptr && _gamepad->isConnected())
        _gamepad->disconnect();

    if (_gamepad != nullptr)
        disconnect(_gamepad, &QGamepad::connectedChanged, this, &GamepadHandler::gamePadConnectionChanged);
    _mutex.unlock();

    emit connectionChange({DeviceType::Gamepad, ConnectionStatus::Disconnected, "Disconnected"});
    if(isRunning())
    {
        quit();
        wait();
    }
}

void GamepadHandler::dispose()
{
    disposeInternal();
    disconnect(QGamepadManager::instance(), &QGamepadManager::connectedGamepadsChanged, this, &GamepadHandler::connectedGamepadsChanged);
}

QHash<QString, QVariant>* GamepadHandler::getState()
{
    const QMutexLocker locker(&_mutex);
    return _gamepadState;
}

bool GamepadHandler::isConnected()
{
    return _isConnected;
}
