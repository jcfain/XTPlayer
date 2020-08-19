#ifndef TCODEHANDLER_H
#define TCODEHANDLER_H
#import "../struct/Funscript.h"
#import "../tool/xmath.h"
#import "settingshandler.h"
#include <QMap>

class TCodeHandler
{
public:
    TCodeHandler();
    QString funscriptToTCode(qint64 position, int speed = 0);

    int calculateRange(const char* channel, int value);
    int getchannelMin(const char* channel);
    int getchannelMax(const char* channel);
};

#endif // TCODEHANDLER_H
