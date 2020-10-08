#include "tcodehandler.h"

TCodeHandler::TCodeHandler()
{
}


QString TCodeHandler::funscriptToTCode(qint64 position, int speed, bool inverted)
{
    QMutexLocker locker(&mutex);
    QString tcode = "";

    if (inverted)
    {
        position = XMath::reverseNumber(position, 0, 100);
    }
    char tcodeValueString[4];
    sprintf(tcodeValueString, "%03d", calculateRange("L0", position));
    tcode += "L0";
    tcode += tcodeValueString;
    if (speed != 0)
    {
      tcode += "I";
      tcode += QString::number(speed);
    }

    if(!SettingsHandler::getGamepadEnabled() || SettingsHandler::getLiveMultiplier())
    {
        foreach(const QString axis, _multiplierAxis)
        {
            float multiplierValue = SettingsHandler::getMultiplierValue(axis);
            if (SettingsHandler::getMultiplierChecked(axis) && multiplierValue != 0)
            {
                char tcodeValueString[4];
                int value = XMath::constrain(XMath::randSine(position * multiplierValue), 0, 100);
                //lowMin + (highMin-lowMin)*level,lowMax + (highMax-lowMax)*level
                //LogHandler::Debug("randSine: "+ QString::number(value));
                if (inverted)
                {
                    //LogHandler::Debug("inverted: "+ QString::number(value));
                    value = XMath::reverseNumber(value, 0, 100);
                }
                sprintf(tcodeValueString, "%03d", calculateRange(axis.toUtf8(), value));
                tcode += " ";
                tcode += axis;
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
                tcode += axis;
                tcode += "500S1000";
            }
        }
    }
    return tcode;
}
int TCodeHandler::calculateRange(const char* channel, int rawValue)
{
    int xMax = SettingsHandler::getAxis(channel).UserMax;
    int xMin = SettingsHandler::getAxis(channel).UserMin;
    // Update for live x range switch
    if(QString(channel) == axisNames.TcXUpDownL0)
    {
        xMax = SettingsHandler::getLiveXRangeMax();
        xMin = SettingsHandler::getLiveXRangeMin();
    }
    int xMid = qRound((xMax + xMin) / 2.0);
    return XMath::mapRange(rawValue, 50, 100, xMid, xMax);
}
