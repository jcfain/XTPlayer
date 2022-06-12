#include "deohandler.h"

DeoHandler::DeoHandler(QObject *parent) :
    VRDeviceHandler(parent)
{
}

DeoHandler::~DeoHandler()
{
    _isConnected = false;
    if (tcpSocket != nullptr)
        delete tcpSocket;
//    if (keepAliveTimer != nullptr)
//        delete keepAliveTimer;
    if (currentPacket != nullptr)
        delete currentPacket;
}

void DeoHandler::init(NetworkAddress address, int waitTimeout)
{
    qRegisterMetaType<ConnectionChangedSignal>();
    qRegisterMetaType<VRPacket>();
    emit connectionChange({DeviceType::Deo, ConnectionStatus::Connecting, "Waiting..."});
    _waitTimeout = waitTimeout;
    _address = address;

    QHostAddress addressObj;
    addressObj.setAddress(_address.address);
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::stateChanged, this, &DeoHandler::onSocketStateChange);
    connect(tcpSocket, &QTcpSocket::errorOccurred, this, &DeoHandler::tcpErrorOccured);
    tcpSocket->connectToHost(addressObj, _address.port);
    currentPacket = new VRPacket
    {
        nullptr,
        0,
        0,
        0,
        0
    };
}

void DeoHandler::sendKeepAlive()
{
    if (_isConnected)
    {
        LogHandler::Debug("Sending keepalive: "+ QString::number(QTime::currentTime().msecsSinceStartOfDay()));
        send(nullptr);
    }
    else
    {
        keepAliveTimer->stop();
    }
}

void DeoHandler::send(const QString &command)
{
    _sendCommand = command;
    if (command != nullptr)
    {
        LogHandler::Debug("Sending to Deo: "+command);
        QByteArray currentRequest("\0\0\0");
        currentRequest.append(command.toUtf8());
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

void DeoHandler::dispose()
{
    LogHandler::Debug("Deo: dispose");
    tearDown();
    emit connectionChange({DeviceType::Deo, ConnectionStatus::Disconnected, "Disconnected"});
}

void DeoHandler::tearDown()
{

    _isConnected = false;
    _isPlaying = false;
    if (keepAliveTimer != nullptr)
        disconnect(keepAliveTimer, &QTimer::timeout, this, &DeoHandler::sendKeepAlive);
    if (keepAliveTimer != nullptr && keepAliveTimer->isActive())
        keepAliveTimer->stop();
    if (tcpSocket != nullptr)
    {
        disconnect(tcpSocket, &QTcpSocket::stateChanged, this, &DeoHandler::onSocketStateChange);
        disconnect(tcpSocket, &QTcpSocket::errorOccurred, this, &DeoHandler::tcpErrorOccured);
        if (tcpSocket->isOpen())
            tcpSocket->disconnectFromHost();
    }
}

void DeoHandler::readData()
{
    QByteArray datagram = tcpSocket->readAll();
    QByteArray header = datagram.remove(0, 4);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(datagram, &error);
    if (doc.isNull())
    {
        LogHandler::Error("Settings json response error: "+error.errorString());
        LogHandler::Error("datagram: "+datagram);
        //emit connectionChange({DeviceType::Deo, ConnectionStatus::Error, "Read error: " + error.errorString()});
//        if(currentPacket != nullptr)
//        {
//            currentPacket->playing = false;
//        }
    }
    else
    {
        QJsonObject jsonObject = doc.object();
        QString path = jsonObject["path"].toString();
        qint64 duration = jsonObject["duration"].toDouble() * 1000;
        qint64 currentTime = jsonObject["currentTime"].toDouble() * 1000;
        float playbackSpeed = jsonObject["playbackSpeed"].toDouble() * 1.0;
        bool playing = jsonObject["playerState"].toInt() == 0; // 0 == true? right? I know...
//        LogHandler::Debug("Deo path: "+path);
//        LogHandler::Debug("Deo duration: "+QString::number(duration));
//        LogHandler::Debug("Deo currentTime------------------------------------------------> "+QString::number(currentTime));
//        LogHandler::Debug("Deo _currentTime------------------------------------------------> "+QString::number(_currentTime));
//        LogHandler::Debug("Deo playbackSpeed: "+QString::number(playbackSpeed));
//        LogHandler::Debug("Deo playing: "+QString::number(playing));
        _mutex.lock();
        currentPacket = new VRPacket
        {
            path,
            duration,
            currentTime,
            playbackSpeed,
            playing
       };
        _isPlaying = playing;
        _currentTime = currentTime;
        //LogHandler::Debug("Deo _isPlaying: "+QString::number(_isPlaying));
        _mutex.unlock();
        emit messageRecieved(*currentPacket);

    }
}

bool DeoHandler::isConnected()
{
    return _isConnected;
}
bool DeoHandler::isPlaying()
{
    const QMutexLocker locker(&_mutex);
    //LogHandler::Error("DeoHandler::isPlaying(): "+ QString::number(_isPlaying));
    return _isPlaying;
}
//void DeoHandler::togglePause()
//{
//    bool isPaused = false;
//    if(!getCurrentDeoPacket()->playing)
//    {
//        isPaused = true;
//    }
//    QJsonObject pausePacket{
//        {"playerState",isPaused}
//    };
//    QJsonDocument jsonResponse = QJsonDocument(pausePacket);
//    send(QString::fromLatin1(jsonResponse.toJson()));
//}
VRPacket DeoHandler::getCurrentPacket()
{
    const QMutexLocker locker(&_mutex);
    VRPacket blankPacket = {
        NULL,
        0,
        0,
        0,
        0
    };
    return (currentPacket == nullptr) ? blankPacket : *currentPacket;
}

void DeoHandler::onSocketStateChange (QAbstractSocket::SocketState state)
{
    //const QMutexLocker locker(&_mutex);
    switch(state) {
        case QAbstractSocket::SocketState::ConnectedState:
        {
            //_mutex.lock();
            _isConnected = true;
            LogHandler::Debug("Deo connected");
            send(nullptr);
            if (keepAliveTimer != nullptr && keepAliveTimer->isActive())
                keepAliveTimer->stop();
            keepAliveTimer = new QTimer(this);
            //_mutex.unlock();
            connect(keepAliveTimer, &QTimer::timeout, this, &DeoHandler::sendKeepAlive);
            keepAliveTimer->start(1000);
            connect(tcpSocket, &QTcpSocket::readyRead, this, &DeoHandler::readData);
            emit connectionChange({DeviceType::Deo, ConnectionStatus::Connected, "Connected"});
            break;
        }
        case QAbstractSocket::SocketState::UnconnectedState:
        {
//            //_mutex.lock();
//            _isConnected = false;
//            _isPlaying = false;
//            //_mutex.unlock();
//            if (keepAliveTimer != nullptr)
//            {
//                disconnect(keepAliveTimer, &QTimer::timeout, this, &DeoHandler::sendKeepAlive);
//            }
//            if (keepAliveTimer != nullptr && keepAliveTimer->isActive())
//            {
//                keepAliveTimer->stop();
//            }
            if(SettingsHandler::getDeoEnabled())
            {
                LogHandler::Debug("DeoVR retrying: " + _address.address);
//                LogHandler::Debug("port: " + QString::number(_address.port));
//                QHostAddress addressObj;
//                addressObj.setAddress(_address.address);
//                tcpSocket->connectToHost(addressObj, _address.port);
                tearDown();
                QTimer::singleShot(2000, [this] () {
                    init(_address, _waitTimeout);
                });
            }
            else
            {
                LogHandler::Debug("Deo disconnected");
                emit connectionChange({DeviceType::Deo, ConnectionStatus::Disconnected, "Disconnected"});
            }
            break;
        }
        case QAbstractSocket::SocketState::ConnectingState:
        {
            LogHandler::Debug("Deo connecting");
            emit connectionChange({DeviceType::Deo, ConnectionStatus::Connecting, "Waiting..."});
            break;
        }
        case QAbstractSocket::SocketState::BoundState:
        {
            LogHandler::Debug("Deo bound");
            break;
        }
        case QAbstractSocket::SocketState::ListeningState:
        {
            LogHandler::Debug("Deo listening");
            break;
        }
        case QAbstractSocket::SocketState::HostLookupState:
        {
            LogHandler::Debug("Deo host look up");
            break;
        }
        case QAbstractSocket::SocketState::ClosingState:
        {
            LogHandler::Debug("Deo closing");
            break;
        }
    }
}

void DeoHandler::tcpErrorOccured(QAbstractSocket::SocketError state)
{

    switch(state)
    {
        case QAbstractSocket::SocketError::AddressInUseError:
        {
            LogHandler::Error("Deo AddressInUseError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ConnectionRefusedError:
        {
            LogHandler::Error("Deo ConnectionRefusedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::DatagramTooLargeError:
        {
            LogHandler::Error("Deo DatagramTooLargeError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::HostNotFoundError:
        {
            LogHandler::Error("Deo HostNotFoundError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::NetworkError:
        {
            LogHandler::Error("Deo NetworkError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::OperationError:
        {
            LogHandler::Error("Deo OperationError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyAuthenticationRequiredError:
        {
            LogHandler::Error("Deo ProxyAuthenticationRequiredError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyConnectionClosedError:
        {
            LogHandler::Error("Deo ProxyConnectionClosedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyConnectionRefusedError:
        {
            LogHandler::Error("Deo ProxyConnectionRefusedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyConnectionTimeoutError:
        {
            LogHandler::Error("Deo ProxyConnectionTimeoutError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyNotFoundError:
        {
            LogHandler::Error("Deo ProxyNotFoundError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyProtocolError:
        {
            LogHandler::Error("Deo ProxyProtocolError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::RemoteHostClosedError:
        {
            LogHandler::Error("Deo RemoteHostClosedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SocketAccessError:
        {
            LogHandler::Error("Deo SocketAccessError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SocketAddressNotAvailableError:
        {
            LogHandler::Error("Deo SocketAddressNotAvailableError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SocketResourceError:
        {
            LogHandler::Error("Deo SocketResourceError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SocketTimeoutError:
        {
            LogHandler::Error("Deo SocketTimeoutError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SslHandshakeFailedError:
        {
            LogHandler::Error("Deo SslHandshakeFailedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SslInternalError:
        {
            LogHandler::Error("Deo SslInternalError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SslInvalidUserDataError:
        {
            LogHandler::Error("Deo SslInvalidUserDataError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::TemporaryError:
        {
            LogHandler::Error("Deo TemporaryError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::UnfinishedSocketOperationError:
        {
            LogHandler::Error("Deo UnfinishedSocketOperationError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::UnknownSocketError:
        {
            LogHandler::Error("Deo UnknownSocketError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::UnsupportedSocketOperationError:
        {
            LogHandler::Error("Deo UnsupportedSocketOperationError: "+tcpSocket->errorString());
            break;
        }
    }
}
//#include <QNetworkRequest>
//#include <QNetworkAccessManager>
//#include <QNetworkReply>
//#include <QEventLoop>
//QString m_device; //!< Device uuid.
//QNetworkReply::NetworkError m_error = QNetworkReply::NoError; //!< Network error.
//QNetworkAccessManager* m_naMgr = nullptr; //!< The current netword access manager. see CActionManager (QNetworkAccessManager* naMgr, QObject* parent).

//static int m_elapsedTime; //!< The time to execute the last action.
//static QString m_lastError; //!< The error generated by the last action.
//bool DeoHandler::post (QString const & device, QUrl const & url, int timeout)
//{
//    if (m_naMgr == nullptr)
//    {
//      m_naMgr = new QNetworkAccessManager (this);
//    }
//  m_lastError.clear ();
//  m_device     = device;
//  bool success = false;
//  if (!isRunning ())
//  {
//    QNetworkRequest req (url);
//    req.setPriority (QNetworkRequest::HighPriority);
//     // To fix a problem with DSM6 (Synology). If User-Agent exists DSM send only the full precision
//     // image not the thumbnails.
//    req.setHeader (QNetworkRequest::UserAgentHeader, " ");
//    req.setHeader (QNetworkRequest::ContentTypeHeader, QString ("text/xml; charset=\"utf-8\""));
//    req.setRawHeader ("Accept-Encoding", "*");
//    req.setRawHeader ("Accept-Language", "*");
//    req.setRawHeader ("Connection", "Close");
//    QString const & actionName    = info.actionName ();
//    QString         soapActionHdr = QString ("\"%1#%2\"").arg (info.serviceID (), actionName);
//    req.setRawHeader ("SOAPAction", soapActionHdr.toUtf8 ());

//    QTime time;
//    time.start ();

//    m_naMgr->setNetworkAccessible (QNetworkAccessManager::Accessible);
//    QNetworkReply* reply = m_naMgr->post (req, info.message ().toUtf8 ());
//    connect (reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
//    connect (reply, SIGNAL(finished()), this, SLOT(finished()));

//    m_error     = QNetworkReply::NoError;
//    int idTimer = startTimer (timeout);
//    int retCode = exec (QEventLoop::ExcludeUserInputEvents/* | QEventLoop::ExcludeSocketNotifiers*/);
//    success     =  retCode == 0 && m_error == QNetworkReply::NoError;
//    killTimer (idTimer);
//    if (success)
//    {
//      info.setResponse (reply->readAll ());
//    }
//    else
//    {
//      if (m_error != QNetworkReply::NoError)
//      {
//        qint32 statusCode = reply->attribute (QNetworkRequest::HttpStatusCodeAttribute).toInt ();
//        m_lastError       = reply->attribute (QNetworkRequest::HttpReasonPhraseAttribute).toString ();
//        QString message   = QString ("Action failed. Response from the server: %1, %2").arg (statusCode).arg (m_lastError);
//        message          += '\n';
//        message          += url.toString () + '\n';
//        message          += info.message () + "\n\n";
//        qDebug () << "CActionManager::post:" << message;
//        m_lastError.prepend (QString ("(%1) ").arg (statusCode));
//      }
//    }

//    reply->deleteLater ();
//    m_elapsedTime = time.elapsed ();
//  }

//  return success;
//}
