#include "tcodehandler.h"

TCodeHandler::TCodeHandler()
{

}

QString TCodeHandler::funscriptToTCode(qint64 position)
{
    char buffer[15] = "";
    int tcodValue = XMath::mapRange(position, 50, 100, 500, 1000);
    char tcodeValueString[4];
    sprintf(tcodeValueString, "%03d", calculateRange("L0", tcodValue));
    strcat (buffer, tcodeValueString);
    if (SettingsHandler::speed > 0) {
      char speed_string[5];
      sprintf(speed_string, "%04d", SettingsHandler::speed);
      strcat (buffer, "S");
      strcat (buffer, speed_string);
    }
    return "L0" + QString(buffer);
}

int TCodeHandler::calculateRange(const char* channel, int value)
{
    return XMath::constrain(value, getchannelMin(channel), getchannelMax(channel));
}

int TCodeHandler::getchannelMin(const char* channel)
{
    if (strcmp(channel, "L0") == 0)
    {
        return SettingsHandler::xMin;
    }
    else if (strcmp(channel, "R1") == 0)
    {
        return SettingsHandler::yRollMin;
    }
    else if (strcmp(channel, "R2") == 0)
    {
        return SettingsHandler::xRollMin;
    }
    return 1;
}

int TCodeHandler::getchannelMax(const char* channel)
{
    if (strcmp(channel, "L0") == 0)
    {
        return SettingsHandler::xMax;
    }
    else if (strcmp(channel, "R1") == 0)
    {
        return SettingsHandler::yRollMax;
    }
    else if (strcmp(channel, "R2") == 0)
    {
        return SettingsHandler::xRollMax;
    }
    return 1000;
}
