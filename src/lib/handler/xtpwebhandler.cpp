#include "xtpwebhandler.h"

XTPWebHandler::XTPWebHandler(QObject *parent) :
    VRDeviceHandler(parent)
{
}

XTPWebHandler::~XTPWebHandler()
{
    _isConnected = false;
//    if (keepAliveTimer != nullptr)
//        delete keepAliveTimer;
    if (_currentPacket)
        delete _currentPacket;
}

void XTPWebHandler::init(NetworkAddress address, int waitTimeout)
{
}

void XTPWebHandler::init(HttpHandler* httpHandler)
{
    _httpHandler = httpHandler;
    qRegisterMetaType<ConnectionChangedSignal>();
    qRegisterMetaType<VRPacket>();
    emit connectionChange({DeviceType::XTPWeb, ConnectionStatus::Connecting, "Waiting..."});

    _currentPacket = new VRPacket
    {
        nullptr,
        0,
        0,
        0,
        0
    };
    connect(_httpHandler, &HttpHandler::readyRead, this, &XTPWebHandler::readData);
}

void XTPWebHandler::send(const QString &command)
{

}

void XTPWebHandler::dispose()
{
    LogHandler::Debug("XTP Web: dispose");
    _isConnected = false;
    emit connectionChange({DeviceType::XTPWeb, ConnectionStatus::Disconnected, "Disconnected"});
    disconnect(_httpHandler, &HttpHandler::readyRead, this, &XTPWebHandler::readData);
}

void XTPWebHandler::readData(QByteArray data)
{
    if(!_isConnected)
    {
        _isConnected = true;
        emit connectionChange({DeviceType::XTPWeb, ConnectionStatus::Connected, "Connected"});
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (doc.isNull())
    {
        LogHandler::Error("XTP Web json response error: "+error.errorString());
        LogHandler::Error("data: "+data);
    }
    else
    {
        QJsonObject jsonObject = doc.object();
        QString path = jsonObject["path"].toString();
        qint64 duration = jsonObject["duration"].toDouble() * 1000;
        qint64 currentTime = jsonObject["currentTime"].toDouble() * 1000;
        float playbackSpeed = jsonObject["playbackSpeed"].toDouble() * 1.0;
        bool playing = jsonObject["playing"].toBool();
//        LogHandler::Debug("XTP Web path: "+path);
//        LogHandler::Debug("XTP Web duration: "+QString::number(duration));
//        LogHandler::Debug("XTP Web currentTime------------------------------------------------> "+QString::number(currentTime));
//        LogHandler::Debug("XTP Web _currentTime------------------------------------------------> "+QString::number(_currentTime));
//        LogHandler::Debug("XTP Web playbackSpeed: "+QString::number(playbackSpeed));
//        LogHandler::Debug("XTP Web playing: "+QString::number(playing));
        _mutex.lock();
        _currentPacket = new VRPacket
        {
            path,
            duration,
            currentTime,
            playbackSpeed,
            playing
       };
        _isPlaying = playing;
        _currentTime = currentTime;
        //LogHandler::Debug("XTP Web _isPlaying: "+QString::number(_isPlaying));
        _mutex.unlock();
        emit messageRecieved(*_currentPacket);

    }
}

bool XTPWebHandler::isConnected()
{
    return _isConnected;
}

bool XTPWebHandler::isPlaying()
{
    const QMutexLocker locker(&_mutex);
    //LogHandler::Error("DeoHandler::isPlaying(): "+ QString::number(_isPlaying));
    return _isPlaying;
}

VRPacket XTPWebHandler::getCurrentPacket()
{
    const QMutexLocker locker(&_mutex);
    VRPacket blankPacket = {
        NULL,
        0,
        0,
        0,
        0
    };
    return (_currentPacket == nullptr) ? blankPacket : *_currentPacket;
}
