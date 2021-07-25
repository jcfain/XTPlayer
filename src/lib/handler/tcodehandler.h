#ifndef TCODEHANDLER_H
#define TCODEHANDLER_H
#include "../struct/Funscript.h"
#include "../struct/ChannelModel.h"
#include "../struct/GamepadState.h"
#include "../lookup/tcodechannellookup.h"
#include "../lookup/AxisNames.h"
#include "../tool/xmath.h"
#include "settingshandler.h"
#include "funscripthandler.h"
#include "loghandler.h"
#include <QMap>
#include <QPair>

class TCodeHandler
{
public:
    TCodeHandler();
    QString funscriptToTCode(std::shared_ptr<FunscriptAction> action, QMap<QString, std::shared_ptr<FunscriptAction>> otherActions);

    int calculateRange(const char* channel, int rawValue);
    QString getRunningHome();
    QString getAllHome();
    QString getSwitchedHome();

private:
    QMutex mutex;
    void getChannelHome(ChannelModel channel, QString &tcode);
    int getTCodePadding();
};

#endif // TCODEHANDLER_H
