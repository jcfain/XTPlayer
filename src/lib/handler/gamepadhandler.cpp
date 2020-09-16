#include "gamepadhandler.h"
#include <QWindow>

GamepadHandler::GamepadHandler(QObject *parent) :
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
    _gamepadState = new GamepadState();
    while(!_stop)
    {
        _mutex.lock();
        _gamepadState->axisLeftX = calculateDeadZone(_gamepad->axisLeftX());
        _gamepadState->axisLeftY = calculateDeadZone(_gamepad->axisLeftY());
        _gamepadState->axisRightX = calculateDeadZone(_gamepad->axisRightX());
        _gamepadState->axisRightY = calculateDeadZone(_gamepad->axisLeftX());
        _gamepadState->buttonA = _gamepad->buttonA();
        _gamepadState->buttonB = _gamepad->buttonB();
        _gamepadState->buttonX = _gamepad->buttonX();
        _gamepadState->buttonY = _gamepad->buttonY();
        _gamepadState->buttonL1 = _gamepad->buttonL1();
        _gamepadState->buttonL2 = _gamepad->buttonL2();
        _gamepadState->buttonL3 = _gamepad->buttonL3();
        _gamepadState->buttonR1 = _gamepad->buttonR1();
        _gamepadState->buttonR2 = _gamepad->buttonR2();
        _gamepadState->buttonR3 = _gamepad->buttonR3();
        _gamepadState->buttonUp = _gamepad->buttonUp();
        _gamepadState->buttonDown = _gamepad->buttonDown();
        _gamepadState->buttonLeft = _gamepad->buttonLeft();
        _gamepadState->buttonRight = _gamepad->buttonRight();
        _gamepadState->buttonSelect = _gamepad->buttonSelect();
        _gamepadState->buttonStart = _gamepad->buttonStart();
        _gamepadState->buttonCenter = _gamepad->buttonCenter();
        _gamepadState->buttonGuide = _gamepad->buttonGuide();
        _mutex.unlock();

//        if (!_stop)
//        {
//            _mutex.lock();
//            _cond.wait(&_mutex);
//            _mutex.unlock();
//        }
    }
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
    disconnect(_gamepad, &QGamepad::connectedChanged, this, &GamepadHandler::gamePadConnectionChanged);
    if(isRunning())
    {
        quit();
        wait();
    }
}

GamepadState* GamepadHandler::getState()
{
    const QMutexLocker locker(&_mutex);
    return _gamepadState;
}

bool GamepadHandler::isConnected()
{
    return _isConnected;
}
