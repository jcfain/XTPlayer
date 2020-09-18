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
    _isConnected = false;
    emit connectionChange({DeviceType::Gamepad, ConnectionStatus::Connecting, "Connecting..."});
    connect(QGamepadManager::instance(), &QGamepadManager::connectedGamepadsChanged, this, &GamepadHandler::connectedGamepadsChanged);
}

void GamepadHandler::connectedGamepadsChanged()
{
    if(!_initialized)
    {
        _gamepads = QGamepadManager::instance()->connectedGamepads();
        LogHandler::Debug("Gamepad connected: "+ QString::number(*_gamepads.begin()));
        _initialized = true;
        _isConnected = true;
        _stop = false;
        _gamepad = new QGamepad(*_gamepads.begin(), this);
        emit connectionChange({DeviceType::Gamepad, ConnectionStatus::Connected, "Connected"});
        connect(_gamepad, &QGamepad::connectedChanged, this, &GamepadHandler::gamePadConnectionChanged);
        start();
    }
}

void GamepadHandler::gamePadConnectionChanged(bool connected)
{
    //_isConnected = connected;
    if(connected)
    {
        LogHandler::Debug("Gamepad connected");
//        _stop = false;
//        emit connectionChange({DeviceType::Gamepad, ConnectionStatus::Connected, "Connected"});
//        start();
    }
    else
    {
        LogHandler::Debug("Gamepad disconnected");
        emit connectionChange({DeviceType::Gamepad, ConnectionStatus::Disconnected, "Disconnected"});
        dispose();
    }
}

void GamepadHandler::run()
{
    QString lastTCode;
    TCodeFactory* tcodeFactory = new TCodeFactory(0.0, 1.0);
    QVector<ChannelValueModel> axisValues;
    //_gamepadState = new QHash<QString, QVariant>();
    GamepadAxisNames gamepadAxisNames;
    while(!_stop)
    {
        _mutex.lock();
//        _gamepadState->insert(gamepadAxisNames.LeftXAxis, calculateDeadZone(_gamepad->axisLeftX()));
//        _gamepadState->insert(gamepadAxisNames.LeftYAxis, calculateDeadZone(_gamepad->axisLeftY()));
//        _gamepadState->insert(gamepadAxisNames.RightXAxis, calculateDeadZone(_gamepad->axisRightX()));
//        _gamepadState->insert(gamepadAxisNames.RightYAxis, calculateDeadZone(_gamepad->axisRightY()));
//        _gamepadState->insert(gamepadAxisNames.A, _gamepad->buttonA());
//        _gamepadState->insert(gamepadAxisNames.B, _gamepad->buttonB());
//        _gamepadState->insert(gamepadAxisNames.X, _gamepad->buttonX());
//        _gamepadState->insert(gamepadAxisNames.Y, _gamepad->buttonY());
//        _gamepadState->insert(gamepadAxisNames.LeftBumper, _gamepad->buttonL1());
//        _gamepadState->insert(gamepadAxisNames.LeftTrigger, _gamepad->buttonL2());
//        _gamepadState->insert(gamepadAxisNames.LeftAxisButton, _gamepad->buttonL3());
//        _gamepadState->insert(gamepadAxisNames.RightBumper, _gamepad->buttonR1());
//        _gamepadState->insert(gamepadAxisNames.RightTrigger, _gamepad->buttonR2());
//        _gamepadState->insert(gamepadAxisNames.RightAxisButton, _gamepad->buttonR3());
//        _gamepadState->insert(gamepadAxisNames.DPadUp, _gamepad->buttonUp());
//        _gamepadState->insert(gamepadAxisNames.DPadDown, _gamepad->buttonDown());
//        _gamepadState->insert(gamepadAxisNames.DPadLeft, _gamepad->buttonLeft());
//        _gamepadState->insert(gamepadAxisNames.DPadRight, _gamepad->buttonRight());
//        _gamepadState->insert(gamepadAxisNames.Select, _gamepad->buttonSelect());
//        _gamepadState->insert(gamepadAxisNames.Start, _gamepad->buttonStart());
//        _gamepadState->insert(gamepadAxisNames.Center, _gamepad->buttonCenter());
//        _gamepadState->insert(gamepadAxisNames.Guide, _gamepad->buttonGuide());var axisValues = new HashSet<ChannelValueModel>();
        axisValues.clear();
        tcodeFactory->calculate(gamepadAxisNames.LeftXAxis, calculateDeadZone(_gamepad->axisLeftX()), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.LeftYAxis, calculateDeadZone(-_gamepad->axisLeftY()), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.RightXAxis, calculateDeadZone(_gamepad->axisRightX()), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.RightYAxis, calculateDeadZone(-_gamepad->axisRightY()), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.RightTrigger, _gamepad->buttonR2(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.LeftTrigger, _gamepad->buttonL2(), axisValues);
        // Binary inputs
        tcodeFactory->calculate(gamepadAxisNames.A, _gamepad->buttonA(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.B, _gamepad->buttonB(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.X, _gamepad->buttonX(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.Y, _gamepad->buttonY(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.RightBumper, _gamepad->buttonR1(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.LeftBumper, _gamepad->buttonL1(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.Start, _gamepad->buttonStart(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.Select, _gamepad->buttonSelect(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.DPadUp, _gamepad->buttonUp(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.DPadDown, _gamepad->buttonDown(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.DPadLeft, _gamepad->buttonLeft(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.DPadRight, _gamepad->buttonRight(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.RightAxisButton, _gamepad->buttonR3(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.LeftAxisButton, _gamepad->buttonL3(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.Center, _gamepad->buttonCenter(), axisValues);
        tcodeFactory->calculate(gamepadAxisNames.Guide, _gamepad->buttonGuide(), axisValues);

        QString currentTCode = tcodeFactory->formatTCode(&axisValues);
        if (lastTCode != currentTCode)
        {
            lastTCode = currentTCode;
            emit emitTCode(currentTCode);
        }
        _mutex.unlock();

//        if (!_stop)
//        {
//            _mutex.lock();
//            _cond.wait(&_mutex);
//            _mutex.unlock();
//        }
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

void GamepadHandler::dispose()
{
    _initialized = false;
    _stop = true;
    _isConnected = false;
    if (_gamepad != nullptr && _gamepad->isConnected())
        _gamepad->disconnect();

    if (_gamepad != nullptr)
        disconnect(_gamepad, &QGamepad::connectedChanged, this, &GamepadHandler::gamePadConnectionChanged);

    emit connectionChange({DeviceType::Gamepad, ConnectionStatus::Disconnected, "Disconnected"});
    if(isRunning())
    {
        quit();
        wait();
    }
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
