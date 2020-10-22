#include "whirligighandler.h"

WhirligigHandler::WhirligigHandler(QObject *parent) :
    VRDeviceHandler(parent)
{
}

WhirligigHandler::~WhirligigHandler()
{
    _isConnected = false;
    if (tcpSocket != nullptr)
        delete tcpSocket;
    if (currentVRPacket != nullptr)
        delete currentVRPacket;
}
void WhirligigHandler::init(NetworkAddress address, int waitTimeout)
{
    qRegisterMetaType<ConnectionChangedSignal>();
    qRegisterMetaType<VRPacket>();
    emit connectionChange({DeviceType::Whirligig, ConnectionStatus::Connecting, "Waiting..."});
    _waitTimeout = waitTimeout;
    _address = address;

    QHostAddress addressObj;
    addressObj.setAddress(_address.address);
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::stateChanged, this, &WhirligigHandler::onSocketStateChange);
    connect(tcpSocket, &QTcpSocket::errorOccurred, this, &WhirligigHandler::tcpErrorOccured);
    tcpSocket->connectToHost(addressObj, _address.port);
}

void WhirligigHandler::send(const QString &command)
{
    _sendCommand = command;
    if (command != nullptr)
    {
        LogHandler::Debug("Sending to Whirligig: "+command);
        QByteArray currentRequest("\0\0\0");
        currentRequest.append(command);
        tcpSocket->write(currentRequest);
        tcpSocket->waitForBytesWritten();
    }
    else
    {
        QByteArray data("\0\0\0");
        tcpSocket->write(data, 4);
        tcpSocket->waitForBytesWritten();
    }
    tcpSocket->flush();
}

void WhirligigHandler::dispose()
{
    LogHandler::Debug("Whirligig: dispose");
    _isConnected = false;
    _isPlaying = false;
    emit connectionChange({DeviceType::Whirligig, ConnectionStatus::Disconnected, "Disconnected"});
    if (tcpSocket != nullptr)
    {
        disconnect(tcpSocket, &QTcpSocket::stateChanged, this, &WhirligigHandler::onSocketStateChange);
        disconnect(tcpSocket, &QTcpSocket::errorOccurred, this, &WhirligigHandler::tcpErrorOccured);
        if (tcpSocket->isOpen())
            tcpSocket->disconnectFromHost();
    }
}

void WhirligigHandler::readData()
{
    //LogHandler::Debug("Whirligig packet recieved");
    QByteArray datagram = tcpSocket->readAll();
    //QByteArray timeDatagram = tcpSocket->readAll();
    QString line = QString::fromUtf8(datagram);
    //LogHandler::Debug("all: "+line);
    bool playing = false;
    qint64 currentTime = 0;
    if (line.contains("S"))
    {
        playing = false;
    }
    if (line.startsWith("C"))
    {
        //LogHandler::Debug("C line: "+line);
        duration = 1;
        auto list = line.split('\n');
        QString pathLine = list[0];
        //QString type = list[1].section(list[1].lastIndexOf(' '), list[1].length()-1);
        duration = list[2].split('=')[1].trimmed().toDouble() * 1000;
        path = pathLine.remove(0, 2).remove(QRegularExpression("[\"\\n]+")).replace("\\\\", "\\").trimmed();
        //LogHandler::Debug("Whirligig path: "+path);
    }
    if (line.startsWith("P"))
    {
        //LogHandler::Debug("P line: "+line);
        playing = true;
        QString timeStamp = line.remove(0, 2).trimmed();
        //LogHandler::Debug("Whirligig timeStamp: "+timeStamp);
        double seconds = timeStamp.toDouble();
        currentTime = seconds * 1000;
        //LogHandler::Debug("Whirligig current time: "+QString::number(currentTime));
    }
    //LogHandler::Debug("Whirligig path: "+path);
    //LogHandler::Debug("Whirligig duration: "+QString::number(duration));
    //LogHandler::Debug("Whirligig currentTime------------------------------------------------> "+QString::number(currentTime));
    //LogHandler::Debug("Whirligig playbackSpeed: "+QString::number(playbackSpeed));
    //LogHandler::Debug("Whirligig playing: "+QString::number(playing));
    _mutex.lock();
    _isPlaying = playing;
    //LogHandler::Debug("Whirligig current time: "+QString::number(currentTime));
    currentVRPacket = new VRPacket
    {
        path,
        duration,
        currentTime,
        0,
        playing
    };
    _mutex.unlock();
    if (time2 - time1 >= 1000)
    {
        time1 = time2;
        //LogHandler::Debug("Whirligig _isPlaying: "+QString::number(_isPlaying));
        emit messageRecieved(*currentVRPacket);
    }
    time2 = (round(mSecTimer.nsecsElapsed() / 1000000));
}

bool WhirligigHandler::isConnected()
{
    return _isConnected;
}
bool WhirligigHandler::isPlaying()
{
    const QMutexLocker locker(&_mutex);
    //LogHandler::Error("WhirligigHandler::isPlaying(): "+ QString::number(_isPlaying));
    return _isPlaying;
}
//void WhirligigHandler::togglePause()
//{
//    bool isPaused = false;
//    if(!getCurrentWhirligigPacket()->playing)
//    {
//        isPaused = true;
//    }
//    QJsonObject pausePacket{
//        {"playerState",isPaused}
//    };
//    QJsonDocument jsonResponse = QJsonDocument(pausePacket);
//    send(QString::fromLatin1(jsonResponse.toJson()));
//}
VRPacket WhirligigHandler::getCurrentPacket()
{
    const QMutexLocker locker(&_mutex);
    VRPacket blankPacket = {
        NULL,
        0,
        0,
        0,
        1
    };
    return (currentVRPacket == nullptr) ? blankPacket : *currentVRPacket;
}

void WhirligigHandler::onSocketStateChange (QAbstractSocket::SocketState state)
{
    //const QMutexLocker locker(&_mutex);
    switch(state) {
        case QAbstractSocket::SocketState::ConnectedState:
        {
            //_mutex.lock();
            _isConnected = true;
            LogHandler::Debug("Whirligig connected");
            //send(nullptr);
            mSecTimer.start();
            connect(tcpSocket, &QTcpSocket::readyRead, this, &WhirligigHandler::readData);
            //if (keepAliveTimer != nullptr && keepAliveTimer->isActive())
                //keepAliveTimer->stop();
            //keepAliveTimer = new QTimer(this);
            //_mutex.unlock();
            //connect(keepAliveTimer, &QTimer::timeout, this, &WhirligigHandler::sendKeepAlive);
            //keepAliveTimer->start(1000);
            emit connectionChange({DeviceType::Whirligig, ConnectionStatus::Connected, "Connected"});
            break;
        }
        case QAbstractSocket::SocketState::UnconnectedState:
        {
            //_mutex.lock();
            _isConnected = false;
            _isPlaying = false;
            //_mutex.unlock();
            //if (keepAliveTimer != nullptr && keepAliveTimer->isActive())
                //keepAliveTimer->stop();
            if(SettingsHandler::getWhirligigEnabled())
            {
                LogHandler::Debug("Whirligig retrying: " + _address.address);
                LogHandler::Debug("port: " + QString::number(_address.port));
                QHostAddress addressObj;
                addressObj.setAddress(_address.address);
                tcpSocket->connectToHost(addressObj, _address.port);
            }
            else
            {
                LogHandler::Debug("Whirligig disconnected");
                emit connectionChange({DeviceType::Whirligig, ConnectionStatus::Disconnected, "Disconnected"});
            }
            break;
        }
        case QAbstractSocket::SocketState::ConnectingState:
        {
            LogHandler::Debug("Whirligig connecting");
            emit connectionChange({DeviceType::Whirligig, ConnectionStatus::Connecting, "Waiting..."});
            break;
        }
        case QAbstractSocket::SocketState::BoundState:
        {
            LogHandler::Debug("Whirligig bound");
            break;
        }
        case QAbstractSocket::SocketState::ListeningState:
        {
            LogHandler::Debug("Whirligig listening");
            break;
        }
        case QAbstractSocket::SocketState::HostLookupState:
        {
            LogHandler::Debug("Whirligig host look up");
            break;
        }
        case QAbstractSocket::SocketState::ClosingState:
        {
            LogHandler::Debug("Whirligig closing");
            break;
        }
    }
}

void WhirligigHandler::tcpErrorOccured(QAbstractSocket::SocketError state)
{

    switch(state)
    {
        case QAbstractSocket::SocketError::AddressInUseError:
        {
            LogHandler::Error("Whirligig AddressInUseError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ConnectionRefusedError:
        {
            LogHandler::Error("Whirligig ConnectionRefusedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::DatagramTooLargeError:
        {
            LogHandler::Error("Whirligig DatagramTooLargeError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::HostNotFoundError:
        {
            LogHandler::Error("Whirligig HostNotFoundError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::NetworkError:
        {
            LogHandler::Error("Whirligig NetworkError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::OperationError:
        {
            LogHandler::Error("Whirligig OperationError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyAuthenticationRequiredError:
        {
            LogHandler::Error("Whirligig ProxyAuthenticationRequiredError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyConnectionClosedError:
        {
            LogHandler::Error("Whirligig ProxyConnectionClosedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyConnectionRefusedError:
        {
            LogHandler::Error("Whirligig ProxyConnectionRefusedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyConnectionTimeoutError:
        {
            LogHandler::Error("Whirligig ProxyConnectionTimeoutError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyNotFoundError:
        {
            LogHandler::Error("Whirligig ProxyNotFoundError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyProtocolError:
        {
            LogHandler::Error("Whirligig ProxyProtocolError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::RemoteHostClosedError:
        {
            LogHandler::Error("Whirligig RemoteHostClosedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SocketAccessError:
        {
            LogHandler::Error("Whirligig SocketAccessError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SocketAddressNotAvailableError:
        {
            LogHandler::Error("Whirligig SocketAddressNotAvailableError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SocketResourceError:
        {
            LogHandler::Error("Whirligig SocketResourceError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SocketTimeoutError:
        {
            LogHandler::Error("Whirligig SocketTimeoutError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SslHandshakeFailedError:
        {
            LogHandler::Error("Whirligig SslHandshakeFailedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SslInternalError:
        {
            LogHandler::Error("Whirligig SslInternalError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SslInvalidUserDataError:
        {
            LogHandler::Error("Whirligig SslInvalidUserDataError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::TemporaryError:
        {
            LogHandler::Error("Whirligig TemporaryError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::UnfinishedSocketOperationError:
        {
            LogHandler::Error("Whirligig UnfinishedSocketOperationError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::UnknownSocketError:
        {
            LogHandler::Error("Whirligig UnknownSocketError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::UnsupportedSocketOperationError:
        {
            LogHandler::Error("Whirligig UnsupportedSocketOperationError: "+tcpSocket->errorString());
            break;
        }
    }
}
