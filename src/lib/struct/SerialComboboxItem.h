#ifndef SERIALCOMBOBOXIREM_H
#define SERIALCOMBOBOXIREM_H
#include <QString>
#include <QSerialPortInfo>
#include <QMetaType>


struct SerialComboboxItem
{
    QString friendlyName;
    QString portName;
};

Q_DECLARE_METATYPE(SerialComboboxItem);

#endif // SERIALCOMBOBOXIREM_H
