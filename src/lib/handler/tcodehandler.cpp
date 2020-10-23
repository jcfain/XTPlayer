#include "tcodehandler.h"

TCodeHandler::TCodeHandler()
{
}


QString TCodeHandler::funscriptToTCode(std::shared_ptr<FunscriptAction> action, QMap<QString, std::shared_ptr<FunscriptAction>> otherActions)
{
    QMutexLocker locker(&mutex);
    QString tcode = "";

    if(action != nullptr)
    {
        int position = action->pos;
        int speed = action->speed;
        if (FunscriptHandler::getInverted())
        {
            position = XMath::reverseNumber(position, 0, 100);
        }
        char tcodeValueString[4];
        sprintf(tcodeValueString, "%03d", calculateRange(axisNames.TcYUpDownL0.toUtf8(), position));
        tcode += axisNames.TcYUpDownL0;
        tcode += tcodeValueString;
        if (speed > 0)
        {
          tcode += "I";
          tcode += QString::number(speed);
        }
    }
    if(otherActions.keys().length() > 0)
    {
        foreach(auto axis, axisNames.BasicAxis)
        {
            if (axis.first == axisNames.TcYUpDownL0)
                continue;
            if (otherActions.contains(axis.first))
            {
                std::shared_ptr<FunscriptAction> axisAction = otherActions.value(axis.first);
                char tcodeValueString[4];
                sprintf(tcodeValueString, "%03d", calculateRange(axis.first.toUtf8(), axisAction->pos));
                tcode += " ";
                tcode += axis.first;
                tcode += tcodeValueString;
                if (axisAction->speed > 0)
                {
                  tcode += "I";
                  tcode += QString::number(axisAction->speed);
                }
            }
        }
    }

    if(action != nullptr && SettingsHandler::getMultiplierEnabled())
    {
        int position = action->pos;
        int speed = action->speed;
        if (FunscriptHandler::getInverted())
        {
            position = XMath::reverseNumber(position, 0, 100);
        }
        foreach(auto axis, axisNames.BasicAxis)
        {
            if (axis.first == axisNames.TcYUpDownL0)
                continue;
            if (otherActions.contains(axis.first))
                continue;
            float multiplierValue = SettingsHandler::getMultiplierValue(axis.first);
            if (SettingsHandler::getMultiplierChecked(axis.first) && multiplierValue != 0.0)
            {
                int value = XMath::constrain(XMath::randSine(position * multiplierValue), 0, 100);
                //lowMin + (highMin-lowMin)*level,lowMax + (highMax-lowMax)*level
                //LogHandler::Debug("randSine: "+ QString::number(value));
                if (FunscriptHandler::getInverted())
                {
                    //LogHandler::Debug("inverted: "+ QString::number(value));
                    value = XMath::reverseNumber(value, 0, 100);
                }
                char tcodeValueString[4];
                sprintf(tcodeValueString, "%03d", calculateRange(axis.first.toUtf8(), value));
                tcode += " ";
                tcode += axis.first;
                tcode += tcodeValueString;
                if (speed > 0)
                {
                  tcode += "S";
                  tcode += QString::number(speed);
                }
            }
            else
            {
                tcode += " ";
                tcode += axis.first;
                tcode += "500S1000";
            }
        }
    }

//    if(action != nullptr && !SettingsHandler::getLiveGamepadConnected())
//    {
//        foreach(auto axis, axisNames.BasicAxis)
//        {
//            if(!tcode.contains(axis.first))
//            {
//                tcode += " ";
//                tcode += axis.first;
//                tcode += "500S1000";
//            }
//        }
//    }
    return tcode.isEmpty() ? nullptr : tcode;
}
int TCodeHandler::calculateRange(const char* channel, int rawValue)
{
    int xMax = SettingsHandler::getAxis(channel).UserMax;
    int xMin = SettingsHandler::getAxis(channel).UserMin;
    // Update for live x range switch
    if(QString(channel) == axisNames.TcYUpDownL0)
    {
        xMax = SettingsHandler::getLiveXRangeMax();
        xMin = SettingsHandler::getLiveXRangeMin();
    }
    int xMid = qRound((xMax + xMin) / 2.0);
    return XMath::mapRange(rawValue, 50, 100, xMid, xMax);
}

QString TCodeHandler::getHome()
{
    QString tcode;
    foreach(auto axis, axisNames.BasicAxis)
    {
        if(axis.first == axisNames.TcYUpDownL0)
            continue;
        tcode += " ";
        tcode += axis.first;
        tcode += "500S500";
    }
    return tcode;
}
