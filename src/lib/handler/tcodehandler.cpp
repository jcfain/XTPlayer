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
        if (FunscriptHandler::getInverted() || SettingsHandler::getChannelInverseChecked(axisNames.Stroke))
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
                int position = axisAction->pos;
                if (axisModel.Inverted)
                {
                    position = XMath::reverseNumber(position, 0, 100);
                }
                char tcodeValueString[4];
                sprintf(tcodeValueString, "%03d", calculateRange(axis.toUtf8(), position));
                if(!tcode.isEmpty())
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
        foreach(auto axis, axisKeys)
        {
            ChannelModel channel = availibleAxis->value(axis);
            if (channel.Dimension == AxisDimension::Heave || channel.Type == AxisType::HalfRange || channel.Type == AxisType::None)
                continue;
            if (SettingsHandler::getFunscriptLoaded(axis))
                continue;
            float multiplierValue = SettingsHandler::getMultiplierValue(axis);
            if (multiplierValue == 0.0)
                continue;
            if (SettingsHandler::getMultiplierChecked(axis))
            {
                auto currentAction = action;
                if (channel.LinkToRelatedMFS && SettingsHandler::getFunscriptLoaded(channel.RelatedChannel) && otherActions.contains(channel.RelatedChannel))
                    currentAction = otherActions.value(channel.RelatedChannel);
                else if(channel.LinkToRelatedMFS && SettingsHandler::getFunscriptLoaded(channel.RelatedChannel) && !otherActions.contains(channel.RelatedChannel) && channel.RelatedChannel != axisNames.Stroke)
                    continue;
                int distance = currentAction->pos >= currentAction->lastPos ? currentAction->pos - currentAction->lastPos : currentAction->lastPos - currentAction->pos;
                if (distance > 0)
                {
                    int value;
                    if (channel.LinkToRelatedMFS && SettingsHandler::getFunscriptLoaded(channel.RelatedChannel) && otherActions.contains(channel.RelatedChannel))
                    {
                        value = currentAction->pos;
//                        LogHandler::Debug("Channel: "+ axis);
//                        LogHandler::Debug("FriendlyName: "+ channel.FriendlyName);
//                        LogHandler::Debug("RelatedChannel: "+ channel.RelatedChannel);
//                        LogHandler::Debug("RelatedChannel FriendlyName: "+ SettingsHandler::getAxis(channel.RelatedChannel).FriendlyName);
//                        LogHandler::Debug("value: "+ QString::number(value));
//                        LogHandler::Debug("currentAction->pos: "+ QString::number(currentAction->pos));
                    }
                    else if(!channel.LinkToRelatedMFS ||
                            (channel.LinkToRelatedMFS && (channel.RelatedChannel == axisNames.Stroke || !SettingsHandler::getFunscriptLoaded(channel.RelatedChannel))))
                    {
                        //LogHandler::Debug("Multiplier: "+ axis);
                        value = XMath::constrain(XMath::randSine(XMath::mapRange(currentAction->pos, 0, 100, 0, 180) * multiplierValue), 0, 100);
                    }
                    //lowMin + (highMin-lowMin)*level,lowMax + (highMax-lowMax)*level
                    //LogHandler::Debug("randSine: "+ QString::number(value));
                    if ((channel.Inverted && channel.LinkToRelatedMFS) || (SettingsHandler::getChannelInverseChecked(axisNames.Stroke) && !channel.Inverted && !channel.LinkToRelatedMFS) )
                    {
                        //LogHandler::Debug("inverted: "+ QString::number(value));
                        value = XMath::reverseNumber(value, 0, 100);
                    }
                    char tcodeValueString[4];
                    sprintf(tcodeValueString, "%03d", calculateRange(axis.toUtf8(), value));
                    tcode += " ";
                    tcode += axis;
                    tcode += tcodeValueString;
                    tcode += "S";
                    float speedModifierValue = SettingsHandler::getDamperValue(axis);
                    char tcodeSpeedString[4];
                    if (SettingsHandler::getDamperChecked(axis) && speedModifierValue > 0.0 && currentAction->speed > 1000 && distance > 50)
                    {
                        sprintf(tcodeSpeedString, "%03d", qRound(currentAction->speed * speedModifierValue));
                        tcode += tcodeValueString;
                    }
                    else
                    {
                        sprintf(tcodeSpeedString, "%03d", currentAction->speed > 0 ? currentAction->speed : 1000);
                        tcode += tcodeValueString;
                    }
                }
            }
            else
            {
                getChannelHome(channel, tcode);
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

QString TCodeHandler::getRunningHome()
{
    QString tcode;
    auto availibleAxis = SettingsHandler::getAvailableAxis();
    auto axisKeys = availibleAxis->keys();
    foreach(auto axis, axisKeys)
    {
        auto channel = availibleAxis->value(axis);
        if(channel.Dimension == AxisDimension::Heave || channel.Type != AxisType::Range)
            continue;
        getChannelHome(channel, tcode);
    }
    return tcode;
}

QString TCodeHandler::getAllHome()
{
    QString tcode;
    auto availibleAxis = SettingsHandler::getAvailableAxis();
    auto axisKeys = availibleAxis->keys();
    foreach(auto axis, axisKeys)
    {
        auto channel = availibleAxis->value(axis);
        if(channel.Type == AxisType::HalfRange || channel.Type == AxisType::None )
            continue;
        getChannelHome(channel, tcode);
    }
    return tcode;
}

QString TCodeHandler::getSwitchedHome()
{
    QString tcode;
    auto availibleAxis = SettingsHandler::getAvailableAxis();
    auto axisKeys = availibleAxis->keys();
    foreach(auto axis, axisKeys)
    {
        auto channel = availibleAxis->value(axis);
        if(channel.Type != AxisType::Switch )
            continue;
        getChannelHome(channel, tcode);
    }
    return tcode;
}

void TCodeHandler::getChannelHome(ChannelModel channel, QString &tcode)
{
    char tcodeValueString[4];
    sprintf(tcodeValueString, "%03d", channel.Mid);
    if(!tcode.isEmpty())
        tcode += " ";
    tcode += channel.Channel;
    tcode += tcodeValueString;
    tcode += "S500";
}
