#ifndef TCODEHANDLER_H
#define TCODEHANDLER_H
#include "../struct/Funscript.h"
#include "../struct/ChannelModel.h"
#include "../struct/GamepadState.h"
#include "../lookup/AxisNames.h"
#include "../tool/xmath.h"
#include "settingshandler.h"
#include "funscripthandler.h"
#include "loghandler.h"
#include <QMap>

class TCodeHandler
{
public:
    TCodeHandler();
    QString funscriptToTCode(qint64 position, int speed);

    int calculateRange(const char* channel, int rawValue);

private:
    QMutex mutex;
    const QList<QString> _multiplierAxis = {"L1", "L2", "R0", "R1", "R2", "V0"};
    const AxisNames axisNames;
};

#endif // TCODEHANDLER_H
