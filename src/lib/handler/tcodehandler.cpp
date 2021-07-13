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
        int distance = action->pos >= action->lastPos ? action->pos - action->lastPos : action->lastPos - action->pos;
        if(distance > 0)
        {
            int position = action->pos;
            int speed = action->speed;
            if (FunscriptHandler::getInverted() || SettingsHandler::getChannelInverseChecked(axisNames.Stroke))
            {
                position = XMath::reverseNumber(position, 0, 100);
            }
            tcode += axisNames.Stroke;
            tcode += QString::number(calculateRange(axisNames.Stroke.toUtf8(), position)).rightJustified(getTCodePadding(), '0');
            // LogHandler::Debug("Stroke tcode: "+ tcode);
            if (speed > 0)
            {
              tcode += "I";
              tcode += QString::number(speed);
            }
        }
    }
    if(!otherActions.empty())
    {
        foreach(auto axis, axisKeys)
        {
            if(!availibleAxis->contains(axis))
                continue;
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
                if(!tcode.isEmpty())
                    tcode += " ";
                tcode += axis;
                tcode += QString::number(calculateRange(axis.toUtf8(), position)).rightJustified(getTCodePadding(), '0');
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
            if(!availibleAxis->contains(axis))
                continue;
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
                if ((channel.LinkToRelatedMFS && SettingsHandler::getFunscriptLoaded(channel.RelatedChannel) && otherActions.contains(channel.RelatedChannel))) // Establish link to related channel to axis that are NOT stroke.
                    currentAction = otherActions.value(channel.RelatedChannel);
                else if(channel.LinkToRelatedMFS && SettingsHandler::getFunscriptLoaded(channel.RelatedChannel) && !otherActions.contains(channel.RelatedChannel) && channel.RelatedChannel != axisNames.Stroke)
                    continue;
                if(currentAction == nullptr)
                    continue;
                int distance = currentAction->pos >= currentAction->lastPos ? currentAction->pos - currentAction->lastPos : currentAction->lastPos - currentAction->pos;
                if (distance > 0)
                {
                    int value = -1;
                    if ((channel.LinkToRelatedMFS && SettingsHandler::getFunscriptLoaded(channel.RelatedChannel) && otherActions.contains(channel.RelatedChannel)) ||
                            (channel.LinkToRelatedMFS && SettingsHandler::getFunscriptLoaded(channel.RelatedChannel) && channel.RelatedChannel == axisNames.Stroke))
                    {
                        value = currentAction->pos;
//                        LogHandler::Debug("Channel: "+ axis);
//                        LogHandler::Debug("FriendlyName: "+ channel.FriendlyName);
//                        LogHandler::Debug("RelatedChannel: "+ channel.RelatedChannel);
//                        LogHandler::Debug("RelatedChannel FriendlyName: "+ SettingsHandler::getAxis(channel.RelatedChannel).FriendlyName);
//                        LogHandler::Debug("LinkToRelatedMFS value: "+ QString::number(value));
//                        LogHandler::Debug("currentAction->pos: "+ QString::number(currentAction->pos));
//                        LogHandler::Debug("action->pos: "+ QString::number(action->pos));
                    }
                    else
                    {
                        //LogHandler::Debug("Multiplier: "+ axis);
                        value = XMath::constrain(XMath::randSine(XMath::mapRange(currentAction->pos, 0, 100, 0, 180) * multiplierValue), 0, 100);
                    }
                    //lowMin + (highMin-lowMin)*level,lowMax + (highMax-lowMax)*level
                    //LogHandler::Debug("randSine: "+ QString::number(value));
                    if (value < 0)
                    {
                        LogHandler::Warn("Value was less than zero: "+ QString::number(value));
                        continue;
                    }
                    if ((channel.Inverted && channel.LinkToRelatedMFS) || (SettingsHandler::getChannelInverseChecked(axisNames.Stroke) && !channel.Inverted && !channel.LinkToRelatedMFS) )
                    {
                        //LogHandler::Debug("inverted: "+ QString::number(value));
                        value = XMath::reverseNumber(value, 0, 100);
                    }
                    tcode += " ";
                    tcode += axis;
                    tcode += QString::number(calculateRange(axis.toUtf8(), value)).rightJustified(getTCodePadding(), '0');
                    tcode += "S";
                    float speedModifierValue = SettingsHandler::getDamperValue(axis);
                    if (SettingsHandler::getDamperChecked(axis) && speedModifierValue > 0.0)
                    {
                        auto speed = currentAction->speed > 0 ? currentAction->speed : 500;
                        tcode += QString::number(qRound(speed * speedModifierValue));
                    }
                    else
                    {
                        tcode += QString::number((currentAction->speed > 0 ? currentAction->speed : 500));
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
    if(!tcode.isEmpty())
        tcode += " ";
    tcode += channel.Channel;
    tcode += QString::number(channel.Mid).rightJustified(getTCodePadding(), '0');;
    tcode += "S500";
}

int TCodeHandler::getTCodePadding()
{
    return SettingsHandler::getSelectedTCodeVersion() == TCodeVersion::v3 ? 4 : 3;
}
