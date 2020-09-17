#ifndef TCODEHANDLER_H
#define TCODEHANDLER_H
#import "../struct/Funscript.h"
#import "../struct/ChannelModel.h"
#import "../struct/GamepadState.h"
#import "../lookup/AxisNames.h"
#import "../tool/xmath.h"
#import "settingshandler.h"
#import "../../settingsdialog.h"
#include <QMap>

class TCodeHandler
{
public:
    TCodeHandler();
    QString funscriptToTCode(qint64 position, int speed = 0);

    int calculateRange(const char* channel, int rawValue);
    int getchannelMin(const char* channel);
    int getchannelMax(const char* channel);

private:
    static QMutex mutex;
};

#endif // TCODEHANDLER_H
