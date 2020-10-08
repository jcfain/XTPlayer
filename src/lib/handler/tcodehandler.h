#ifndef TCODEHANDLER_H
#define TCODEHANDLER_H
#import "../struct/Funscript.h"
#import "../struct/ChannelModel.h"
#import "../struct/GamepadState.h"
#import "../lookup/AxisNames.h"
#import "../tool/xmath.h"
#import "settingshandler.h"
#include "loghandler.h"
#include <QMap>

class TCodeHandler
{
public:
    TCodeHandler();
    QString funscriptToTCode(qint64 position, int speed, bool inverted);

    int calculateRange(const char* channel, int rawValue);

private:
    QMutex mutex;
    const QList<QString> _multiplierAxis = {"R0", "R1", "R2", "V0"};
    const AxisNames axisNames;
};

#endif // TCODEHANDLER_H
