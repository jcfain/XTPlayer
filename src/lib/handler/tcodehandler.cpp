#include "tcodehandler.h"

TCodeHandler::TCodeHandler()
{
}


QString TCodeHandler::funscriptToTCode(std::shared_ptr<FunscriptAction> action, QMap<QString, std::shared_ptr<FunscriptAction>> otherActions)
{
    QMutexLocker locker(&mutex);
    QString tcode = "";
    auto availibleAxis = SettingsHandler::getAvailableAxis();
    auto axisKeys = availibleAxis->keys();
    if(action != nullptr)
    {
        int position = action->pos;
        int speed = action->speed;
        if (FunscriptHandler::getInverted())
        {
            position = XMath::reverseNumber(position, 0, 100);
        }
        char tcodeValueString[4];
        sprintf(tcodeValueString, "%03d", calculateRange(axisNames.Stroke.toUtf8(), position));
        tcode += axisNames.Stroke;
        tcode += tcodeValueString;
        if (speed > 0)
        {
          tcode += "I";
          tcode += QString::number(speed);
        }
    }
    if(otherActions.keys().length() > 0)
    {
        foreach(auto axis, axisKeys)
        {
            auto axisModel = availibleAxis->value(axis);
            if (axisModel.Channel == axisNames.Stroke || axisModel.TrackName.isEmpty())
                continue;
            if (otherActions.contains(axis))
            {
                std::shared_ptr<FunscriptAction> axisAction = otherActions.value(axis);
                char tcodeValueString[4];
                sprintf(tcodeValueString, "%03d", calculateRange(axis.toUtf8(), axisAction->pos));
                tcode += " ";
                tcode += axis;
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
        foreach(auto axis, axisKeys)
        {
            ChannelModel channel = availibleAxis->value(axis);
            if (channel.Dimension == AxisDimension::Heave || channel.Type == AxisType::HalfRange)
                continue;
            if (otherActions.contains(axis))
                continue;
            float multiplierValue = SettingsHandler::getMultiplierValue(axis);
            if (multiplierValue == 0.0)
                continue;
            if (SettingsHandler::getMultiplierChecked(axis))
            {
                int value = XMath::constrain(XMath::randSine(XMath::mapRange(position, 0, 100, 0, 180) * multiplierValue), 0, 100);
                //lowMin + (highMin-lowMin)*level,lowMax + (highMax-lowMax)*level
                //LogHandler::Debug("randSine: "+ QString::number(value));
                if (FunscriptHandler::getInverted())
                {
                    //LogHandler::Debug("inverted: "+ QString::number(value));
                    value = XMath::reverseNumber(value, 0, 100);
                }
                char tcodeValueString[4];
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
    return tcode.isEmpty() ? nullptr : tcode;
}
int TCodeHandler::calculateRange(const char* channel, int rawValue)
{
    int xMax = SettingsHandler::getAxis(channel).UserMax;
    int xMin = SettingsHandler::getAxis(channel).UserMin;
    // Update for live x range switch
    if(QString(channel) == axisNames.Stroke)
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
    auto availibleAxis = SettingsHandler::getAvailableAxis();
    auto axisKeys = availibleAxis->keys();
    foreach(auto axis, axisKeys)
    {
        auto channel = availibleAxis->value(axis);
        if(channel.Dimension == AxisDimension::Heave ||  channel.Type != AxisType::Range)
            continue;
        tcode += " ";
        tcode += axis;
        tcode += "500S500";
    }
    return tcode;
}
