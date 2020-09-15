#include "tcodehandler.h"

TCodeHandler::TCodeHandler()
{

}

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

    if (SettingsHandler::getYRollMultiplierChecked() && SettingsHandler::getYRollMultiplierValue() !=0)
    {
        char tcodeYRollValueString[4];
        int yRollValue = qRound(position * SettingsHandler::getYRollMultiplierValue());
        sprintf(tcodeYRollValueString, "%03d", calculateRange("R1", yRollValue));
        tcode += " R1";
        tcode += tcodeYRollValueString;
        if (speed != 0) {
          tcode += "I";
          tcode += QString::number(speed);
        }
    }
    else
    {
        tcode += " R1500S1000";
    }

    if (SettingsHandler::getXRollMultiplierChecked() && SettingsHandler::getXRollMultiplierValue() !=0)
    {
        char tcodeXRollValueString[4];
        int xRollValue = qRound(position * SettingsHandler::getXRollMultiplierValue());
        sprintf(tcodeXRollValueString, "%03d", calculateRange("R2", xRollValue));
        tcode += " R2";
        tcode += tcodeXRollValueString;
        if (speed != 0) {
          tcode += "I";
          tcode += QString::number(speed);
        }
    }
    else
    {
        tcode += " R2500S1000";
    }

    if (SettingsHandler::getTwistMultiplierChecked() && SettingsHandler::getTwistMultiplierValue() !=0)
    {
        char tcodeTwistValueString[4];
        int twistValue = qRound(position * SettingsHandler::getTwistMultiplierValue());
        sprintf(tcodeTwistValueString, "%03d", calculateRange("R0", twistValue));
        tcode += " R0";
        tcode += tcodeTwistValueString;
        if (speed != 0) {
          tcode += "I";
          tcode += QString::number(speed);
        }
    }
    else
    {
        tcode += " R0500S1000";
    }

    if (SettingsHandler::getVibMultiplierChecked() && SettingsHandler::getVibMultiplierValue() !=0)
    {
        char tcodeVibValueString[4];
        int vibValue = qRound(position * SettingsHandler::getVibMultiplierValue());
        sprintf(tcodeVibValueString, "%03d", XMath::constrain(vibValue, 1, 1000));
        tcode += " V0";
        tcode += tcodeVibValueString;
        if (speed != 0) {
          tcode += "I";
          tcode += QString::number(speed);
        }
    }
    else
    {
        tcode += " V0500S1000";
    }
    return tcode;
}

int TCodeHandler::calculateRange(const char* channel, int rawValue)
{
    int xMax = getchannelMax(channel);
    int xMin = getchannelMin(channel);
    int xMid = qRound((xMax + xMin) / 2.0);
    return XMath::mapRange(rawValue, 50, 100, xMid, xMax);
}

int TCodeHandler::getchannelMin(const char* channel)
{
    if (strcmp(channel, "L0") == 0)
    {
        return SettingsHandler::getXMin();
    }
    else if (strcmp(channel, "R1") == 0)
    {
        return SettingsHandler::getYRollMin();
    }
    else if (strcmp(channel, "R2") == 0)
    {
        return SettingsHandler::getXRollMin();
    }
    else if (strcmp(channel, "R0") == 0)
    {
        return SettingsHandler::getTwistMin();
    }
    return 1;
}

int TCodeHandler::getchannelMax(const char* channel)
{
    if (strcmp(channel, "L0") == 0)
    {
        return SettingsHandler::getXMax();
    }
    else if (strcmp(channel, "R1") == 0)
    {
        return SettingsHandler::getYRollMax();
    }
    else if (strcmp(channel, "R2") == 0)
    {
        return SettingsHandler::getXRollMax();
    }
    else if (strcmp(channel, "R0") == 0)
    {
        return SettingsHandler::getTwistMax();
    }
    return 1000;
}

QMutex TCodeHandler::mutex;