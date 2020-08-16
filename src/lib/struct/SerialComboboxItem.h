#ifndef SERIALCOMBOBOXIREM_H
#define SERIALCOMBOBOXIREM_H
#include <QString>
#include <QSerialPortInfo>

struct SerialComboboxItem
{
    QString friendlyName;
    QString portName;
    QSerialPortInfo* portInfo;
};

#endif // SERIALCOMBOBOXIREM_H
