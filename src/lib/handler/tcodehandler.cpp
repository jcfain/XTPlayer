#include "tcodehandler.h"

TCodeHandler::TCodeHandler()
{
}

const QList<QString> _multiplierAxis = {"R0", "R1", "R2", "V0"};

QString TCodeHandler::funscriptToTCode(qint64 position, int speed)
{
    QMutexLocker locker(&mutex);
    QString tcode = "";

    char tcodeValueString[4];
    sprintf(tcodeValueString, "%03d", calculateRange("L0", position));
    tcode += "L0";
    tcode += tcodeValueString;
    if (speed != 0) {
      tcode += "I";
      tcode += QString::number(speed);
    }

    if(!SettingsHandler::getGamepadEnabled())
    {
        foreach(QString axis, _multiplierAxis)
        {
            float multiplierValue = SettingsHandler::getMultiplierValue(axis);
            if (SettingsHandler::getMultiplierChecked(axis) && multiplierValue != 0)
            {
                char tcodeValueString[4];
                int value = qRound(position * multiplierValue);
                sprintf(tcodeValueString, "%03d", calculateRange(axis.toUtf8(), value));
                tcode += " ";
                tcode += axis;
                tcode += tcodeValueString;
                if (speed > 0) {
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
    else if(SettingsHandler::getGamepadEnabled())
    {
//        QHash<QString, QVariant>* state = _xSettings->getGamepadHandler()->getState();
//        foreach(QString gamepadAxis, state->keys())
//        {
//            QString tcodeAxis = SettingsHandler::getGamePadButtonMap(gamepadAxis);;
//            if(tcodeAxis != "L0")
//                tcode += " " + gamePadToTCode(tcodeAxis, state->value(gamepadAxis).toDouble());
//        }
    }
    return tcode;
}

AxisNames axisNames;
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

QMutex TCodeHandler::mutex;
