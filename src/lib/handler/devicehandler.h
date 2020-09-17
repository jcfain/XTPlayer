#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H
#include <QTime>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include "settingshandler.h"
#include "loghandler.h"
#include "../lookup/enum.h"
#include "../struct/ConnectionChangedSignal.h"


class DeviceHandler : public QThread
{
    Q_OBJECT

signals:
    void errorOccurred(QString error);
    void connectionChange(ConnectionChangedSignal status);
    void emitTCode(QString tcode);

public:
    explicit DeviceHandler(QObject *parent = nullptr);
    ~DeviceHandler();
    virtual void sendTCode(const QString &tcode);
    virtual void dispose();
    virtual bool isConnected();

private:
    virtual void run() override;
};

#endif // DEVICEHANDLER_H
