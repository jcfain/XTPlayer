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
    QString lastAction;
    TCodeFactory* tcodeFactory = new TCodeFactory(0.0, 1.0);
    QVector<ChannelValueModel> axisValues;
    GamepadAxisNames gamepadAxisNames;
    AxisNames axisNames;
    MediaActions mediaActions;
    bool resetActionTimer = false;
    qint64 timer1 = QTime::currentTime().msecsSinceStartOfDay();
    qint64 timer2 = QTime::currentTime().msecsSinceStartOfDay();
    while(!_stop)
    {
        if (timer2 - timer1 >= 500)
        {
            timer1 = timer2;
            if(resetActionTimer)
            {
                lastAction = nullptr;
                resetActionTimer = false;
            }
        }
        timer2 = QTime::currentTime().msecsSinceStartOfDay();

        axisValues.clear();
        QString LeftXAxis = SettingsHandler::getGamePadMapButton(gamepadAxisNames.LeftXAxis);
        if (!mediaActions.Values.contains(LeftXAxis))
        {
            tcodeFactory->calculate(LeftXAxis, calculateDeadZone(_gamepad->axisLeftX()), axisValues);
        }
        else if(_gamepad->axisLeftX() != 0 && LeftXAxis != axisNames.None && lastAction != LeftXAxis)
        {
            emit emitAction(LeftXAxis);
            lastAction = LeftXAxis;
            resetActionTimer = true;
        }

        QString LeftYAxis = SettingsHandler::getGamePadMapButton(gamepadAxisNames.LeftYAxis);
        if (!mediaActions.Values.contains(LeftYAxis))
            tcodeFactory->calculate(LeftYAxis, calculateDeadZone(-_gamepad->axisLeftY()), axisValues);
        else if(_gamepad->axisLeftY() != 0 && LeftYAxis != axisNames.None && lastAction != LeftYAxis)
        {
            emit emitAction(LeftYAxis);
            lastAction = LeftYAxis;
            resetActionTimer = true;
        }

        QString RightXAxis = SettingsHandler::getGamePadMapButton(gamepadAxisNames.RightXAxis);
        if (!mediaActions.Values.contains(RightXAxis))
            tcodeFactory->calculate(RightXAxis, calculateDeadZone(_gamepad->axisRightX()), axisValues);
        else if(_gamepad->axisRightX() != 0 && RightXAxis != axisNames.None && lastAction != RightXAxis)
        {
            emit emitAction(RightXAxis);
            lastAction = RightXAxis;
            resetActionTimer = true;
        }

        QString RightYAxis = SettingsHandler::getGamePadMapButton(gamepadAxisNames.RightYAxis);
        if (!mediaActions.Values.contains(RightYAxis))
            tcodeFactory->calculate(RightYAxis, calculateDeadZone(-_gamepad->axisRightY()), axisValues);
        else if(_gamepad->axisRightY() != 0 && RightYAxis != axisNames.None && lastAction != RightYAxis)
        {
            emit emitAction(RightYAxis);
            lastAction = RightYAxis;
            resetActionTimer = true;
        }

        QString RightTrigger = SettingsHandler::getGamePadMapButton(gamepadAxisNames.RightTrigger);
        if (!mediaActions.Values.contains(RightTrigger))
            tcodeFactory->calculate(RightTrigger, _gamepad->buttonR2(), axisValues);
        else if(_gamepad->buttonR2() != 0 && RightTrigger != axisNames.None && lastAction != RightTrigger)
        {
            emit emitAction(RightTrigger);
            lastAction = RightTrigger;
            resetActionTimer = true;
        }

        QString LeftTrigger = SettingsHandler::getGamePadMapButton(gamepadAxisNames.LeftTrigger);
        if (!mediaActions.Values.contains(LeftTrigger))
            tcodeFactory->calculate(LeftTrigger, _gamepad->buttonL2(), axisValues);
        else if(_gamepad->buttonL2() != 0 && LeftTrigger != axisNames.None && lastAction != LeftTrigger)
        {
            emit emitAction(LeftTrigger);
            lastAction = LeftTrigger;
            resetActionTimer = true;
        }

        // Binary inputs
        QString A = SettingsHandler::getGamePadMapButton(gamepadAxisNames.A);
        if (!mediaActions.Values.contains(A))
            tcodeFactory->calculate(A, _gamepad->buttonA(), axisValues);
        else if(_gamepad->buttonA() && A != axisNames.None && lastAction != A)
        {
            emit emitAction(LeftXAxis);
            lastAction = A;
            resetActionTimer = true;
        }

        QString B = SettingsHandler::getGamePadMapButton(gamepadAxisNames.B);
        if (!mediaActions.Values.contains(B))
            tcodeFactory->calculate(B, _gamepad->buttonB(), axisValues);
        else if(_gamepad->buttonB() && B != axisNames.None && lastAction != B)
        {
            emit emitAction(B);
            lastAction = B;
            resetActionTimer = true;
        }

        QString X = SettingsHandler::getGamePadMapButton(gamepadAxisNames.X);
        if (!mediaActions.Values.contains(X))
            tcodeFactory->calculate(X, _gamepad->buttonX(), axisValues);
        else if(_gamepad->buttonX() && X != axisNames.None && lastAction != X)
        {
            emit emitAction(X);
            lastAction = X;
            resetActionTimer = true;
        }

        QString Y = SettingsHandler::getGamePadMapButton(gamepadAxisNames.Y);
        if (!mediaActions.Values.contains(Y))
            tcodeFactory->calculate(Y, _gamepad->buttonY(), axisValues);
        else if(_gamepad->buttonY() && Y != axisNames.None && lastAction != Y)
        {
            emit emitAction(Y);
            lastAction = Y;
            resetActionTimer = true;
        }

        QString RightBumper = SettingsHandler::getGamePadMapButton(gamepadAxisNames.RightBumper);
        if (!mediaActions.Values.contains(RightBumper))
            tcodeFactory->calculate(RightBumper, _gamepad->buttonR1(), axisValues);
        else if(_gamepad->buttonR1() && RightBumper != axisNames.None && lastAction != RightBumper)
        {
            emit emitAction(RightBumper);
            lastAction = RightBumper;
            resetActionTimer = true;
        }

        QString LeftBumper = SettingsHandler::getGamePadMapButton(gamepadAxisNames.LeftBumper);
        if (!mediaActions.Values.contains(LeftBumper))
            tcodeFactory->calculate(LeftBumper, _gamepad->buttonL1(), axisValues);
        else if(_gamepad->buttonL1() && LeftBumper != axisNames.None && lastAction != LeftBumper)
        {
            emit emitAction(LeftBumper);
            lastAction = LeftBumper;
            resetActionTimer = true;
        }

        QString Start = SettingsHandler::getGamePadMapButton(gamepadAxisNames.Start);
        if (!mediaActions.Values.contains(Start))
            tcodeFactory->calculate(Start, _gamepad->buttonStart(), axisValues);
        else if(_gamepad->buttonStart() && Start != axisNames.None && lastAction != Start)
        {
            emit emitAction(Start);
            lastAction = Start;
            resetActionTimer = true;
        }

        QString Select = SettingsHandler::getGamePadMapButton(gamepadAxisNames.Select);
        if (!mediaActions.Values.contains(Select))
            tcodeFactory->calculate(Select, _gamepad->buttonSelect(), axisValues);
        else if(_gamepad->buttonSelect() && Select != axisNames.None && lastAction != Select)
        {
            emit emitAction(Select);
            lastAction = Select;
            resetActionTimer = true;
        }

        QString DPadUp = SettingsHandler::getGamePadMapButton(gamepadAxisNames.DPadUp);
        if (!mediaActions.Values.contains(DPadUp))
            tcodeFactory->calculate(DPadUp, _gamepad->buttonUp(), axisValues);
        else if(_gamepad->buttonUp() && DPadUp != axisNames.None && lastAction != DPadUp)
        {
            emit emitAction(DPadUp);
            lastAction = DPadUp;
            resetActionTimer = true;
        }

        QString DPadDown = SettingsHandler::getGamePadMapButton(gamepadAxisNames.DPadDown);
        if (!mediaActions.Values.contains(DPadDown))
            tcodeFactory->calculate(DPadDown, _gamepad->buttonDown(), axisValues);
        else if(_gamepad->buttonDown() && DPadDown != axisNames.None && lastAction != DPadDown)
        {
            emit emitAction(DPadDown);
            lastAction = DPadDown;
            resetActionTimer = true;
        }

        QString DPadLeft = SettingsHandler::getGamePadMapButton(gamepadAxisNames.DPadLeft);
        if (!mediaActions.Values.contains(DPadLeft))
            tcodeFactory->calculate(DPadLeft, _gamepad->buttonLeft(), axisValues);
        else if(_gamepad->buttonLeft() && DPadLeft != axisNames.None && lastAction != DPadLeft)
        {
            emit emitAction(DPadLeft);
            lastAction = DPadLeft;
            resetActionTimer = true;
        }

        QString DPadRight = SettingsHandler::getGamePadMapButton(gamepadAxisNames.DPadRight);
        if (!mediaActions.Values.contains(DPadRight))
            tcodeFactory->calculate(DPadRight, _gamepad->buttonRight(), axisValues);
        else if(_gamepad->buttonRight() && DPadRight != axisNames.None && lastAction != DPadRight)
        {
            emit emitAction(DPadRight);
            lastAction = DPadRight;
            resetActionTimer = true;
        }

        QString RightAxisButton = SettingsHandler::getGamePadMapButton(gamepadAxisNames.RightAxisButton);
        if (!mediaActions.Values.contains(RightAxisButton))
            tcodeFactory->calculate(RightAxisButton, _gamepad->buttonR3(), axisValues);
        else if(_gamepad->buttonR3() && RightAxisButton != axisNames.None && lastAction != RightAxisButton)
        {
            emit emitAction(RightAxisButton);
            lastAction = RightAxisButton;
            resetActionTimer = true;
        }

        QString LeftAxisButton = SettingsHandler::getGamePadMapButton(gamepadAxisNames.LeftAxisButton);
        if (!mediaActions.Values.contains(LeftAxisButton))
            tcodeFactory->calculate(LeftAxisButton, _gamepad->buttonL3(), axisValues);
        else if(_gamepad->buttonL3() && LeftAxisButton != axisNames.None && lastAction != LeftAxisButton)
        {
            emit emitAction(LeftAxisButton);
            lastAction = LeftAxisButton;
            resetActionTimer = true;
        }

        QString Center = SettingsHandler::getGamePadMapButton(gamepadAxisNames.Center);
        if (!mediaActions.Values.contains(Center))
            tcodeFactory->calculate(Center, _gamepad->buttonCenter(), axisValues);
        else if(_gamepad->buttonCenter() && Center != axisNames.None && lastAction != Center)
        {
            emit emitAction(Center);
            lastAction = Center;
            resetActionTimer = true;
        }

        QString Guide = SettingsHandler::getGamePadMapButton(gamepadAxisNames.Guide);
        if (!mediaActions.Values.contains(Guide))
            tcodeFactory->calculate(Guide, _gamepad->buttonGuide(), axisValues);
        else if(_gamepad->buttonGuide() && Guide != axisNames.None && lastAction != Guide)
        {
            emit emitAction(Guide);
            lastAction = Guide;
            resetActionTimer = true;
        }

        QString currentTCode = tcodeFactory->formatTCode(&axisValues);
        if (lastTCode != currentTCode)
        {
            lastTCode = currentTCode;
            emit emitTCode(currentTCode);
        }
    }
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
