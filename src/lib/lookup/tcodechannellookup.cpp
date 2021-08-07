#include "tcodechannellookup.h"

void TCodeChannelLookup::setSelectedTCodeVersion(TCodeVersion version)
{
    _selectedTCodeVersionMap = TCodeVersionMap.value(version);
}
QMap<AxisNames,  QString> TCodeChannelLookup::GetSelectedVersionMap()
{
    return _selectedTCodeVersionMap;
}
QString TCodeChannelLookup::ToString(AxisNames axisName)
{
    return _selectedTCodeVersionMap.value(axisName);
}
void TCodeChannelLookup::AddUserAxis(QString channel)
{
    _selectedTCodeVersionMap.insert((AxisNames)_channelCount, channel);
    _channelCount++;
}
bool TCodeChannelLookup::ChannelExists(QString channel)
{
    return _selectedTCodeVersionMap.values().contains(channel);
}
QString TCodeChannelLookup::None()
{
    return _selectedTCodeVersionMap.value(AxisNames::None);
}
QString TCodeChannelLookup::Stroke()
{
    return _selectedTCodeVersionMap.value(AxisNames::Stroke);
}
QString TCodeChannelLookup::StrokeUp()
{
    return _selectedTCodeVersionMap.value(AxisNames::StrokeUp);
}
QString TCodeChannelLookup::StrokeDown()
{
    return _selectedTCodeVersionMap.value(AxisNames::StrokeDown);
}
QString TCodeChannelLookup::Sway()
{
    return _selectedTCodeVersionMap.value(AxisNames::Sway);
}
QString TCodeChannelLookup::SwayRight()
{
    return _selectedTCodeVersionMap.value(AxisNames::SwayRight);
}
QString TCodeChannelLookup::SwayLeft()
{
    return _selectedTCodeVersionMap.value(AxisNames::SwayLeft);
}
QString TCodeChannelLookup::Surge()
{
    return _selectedTCodeVersionMap.value(AxisNames::Surge);
}
QString TCodeChannelLookup::SurgeForward()
{
    return _selectedTCodeVersionMap.value(AxisNames::SurgeForward);
}
QString TCodeChannelLookup::SurgeBack()
{
    return _selectedTCodeVersionMap.value(AxisNames::SurgeBack);
}
QString TCodeChannelLookup::Twist()
{
    return _selectedTCodeVersionMap.value(AxisNames::Twist);
}
QString TCodeChannelLookup::TwistCounterClockwise()
{
    return _selectedTCodeVersionMap.value(AxisNames::TwistCounterClockwise);
}
QString TCodeChannelLookup::TwistClockwise()
{
    return _selectedTCodeVersionMap.value(AxisNames::TwistClockwise);
}
QString TCodeChannelLookup::Roll()
{
    return _selectedTCodeVersionMap.value(AxisNames::Roll);
}
QString TCodeChannelLookup::RollRight()
{
    return _selectedTCodeVersionMap.value(AxisNames::RollRight);
}
QString TCodeChannelLookup::RollLeft()
{
    return _selectedTCodeVersionMap.value(AxisNames::RollLeft);
}
QString TCodeChannelLookup::Pitch()
{
    return _selectedTCodeVersionMap.value(AxisNames::Pitch);
}
QString TCodeChannelLookup::PitchForward()
{
    return _selectedTCodeVersionMap.value(AxisNames::PitchForward);
}
QString TCodeChannelLookup::PitchBack()
{
    return _selectedTCodeVersionMap.value(AxisNames::PitchBack);
}
QString TCodeChannelLookup::Vib()
{
    return _selectedTCodeVersionMap.value(AxisNames::Vib);
}
QString TCodeChannelLookup::Lube()
{
    return _selectedTCodeVersionMap.value(AxisNames::Lube);
}
QString TCodeChannelLookup::Suck()
{
    return _selectedTCodeVersionMap.value(AxisNames::Suck);
}
QString TCodeChannelLookup::SuckMore()
{
    return _selectedTCodeVersionMap.value(AxisNames::SuckMore);
}
QString TCodeChannelLookup::SuckLess()
{
    return _selectedTCodeVersionMap.value(AxisNames::SuckLess);
}
QString TCodeChannelLookup::SuckPosition()
{
    return _selectedTCodeVersionMap.value(AxisNames::SuckPosition);
}
QString TCodeChannelLookup::SuckMorePosition()
{
    return _selectedTCodeVersionMap.value(AxisNames::SuckMorePosition);
}
QString TCodeChannelLookup::SuckLessPosition()
{
    return _selectedTCodeVersionMap.value(AxisNames::SuckLessPosition);
}

int TCodeChannelLookup::_channelCount = (int)AxisNames::AXIS_NAMES_LENGTH;
QString TCodeChannelLookup::PositiveModifier = "+";
QString TCodeChannelLookup::NegativeModifier = "-";
QString TCodeChannelLookup::NA = "None";
QString TCodeChannelLookup::L0 = "L0";
QString TCodeChannelLookup::L2 = "L2";
QString TCodeChannelLookup::L1 = "L1";
QString TCodeChannelLookup::R0 = "R0";
QString TCodeChannelLookup::R1 = "R1";
QString TCodeChannelLookup::R2 = "R2";
QString TCodeChannelLookup::V0 = "V0";
QString TCodeChannelLookup::V1 = "V1";
QString TCodeChannelLookup::L3 = "L3";
QString TCodeChannelLookup::A0 = "A0";
QString TCodeChannelLookup::A1 = "A1";
QString TCodeChannelLookup::A2 = "A2";
QMap<AxisNames, QString> TCodeChannelLookup::_selectedTCodeVersionMap;
QHash<TCodeVersion, QMap<AxisNames, QString>> TCodeChannelLookup::TCodeVersionMap =
{
    {
        TCodeVersion::v2,
        {
            {AxisNames::None, NA},
            {AxisNames::Stroke, L0},
            {AxisNames::StrokeUp, L0 + PositiveModifier},
            {AxisNames::StrokeDown, L0 + NegativeModifier},
            {AxisNames::Roll, R1},
            {AxisNames::RollRight, R1 + PositiveModifier},
            {AxisNames::RollLeft, R1 + NegativeModifier},
            {AxisNames::Pitch, R2},
            {AxisNames::PitchForward, R2 + PositiveModifier},
            {AxisNames::PitchBack, R2 + NegativeModifier},
            {AxisNames::Twist, R0},
            {AxisNames::TwistClockwise, R0 + PositiveModifier},
            {AxisNames::TwistCounterClockwise, R0 + NegativeModifier},
            {AxisNames::Surge, L1},
            {AxisNames::SurgeForward, L1 + PositiveModifier},
            {AxisNames::SurgeBack, L1 + NegativeModifier},
            {AxisNames::Sway, L2},
            {AxisNames::SwayLeft, L2 + PositiveModifier},
            {AxisNames::SwayRight, L2 + NegativeModifier},
            {AxisNames::Vib, V0},
            {AxisNames::Lube, V1},
            {AxisNames::Suck, L3},
            {AxisNames::SuckMore, L3 + NegativeModifier},
            {AxisNames::SuckLess, L3 + PositiveModifier}
        }
    },
    {
        TCodeVersion::v3,
        {
            {AxisNames::None, NA},
            {AxisNames::Stroke, L0},
            {AxisNames::StrokeUp, L0 + PositiveModifier},
            {AxisNames::StrokeDown, L0 + NegativeModifier},
            {AxisNames::Roll, R1},
            {AxisNames::RollRight, R1 + PositiveModifier},
            {AxisNames::RollLeft, R1 + NegativeModifier},
            {AxisNames::Pitch, R2},
            {AxisNames::PitchForward, R2 + PositiveModifier},
            {AxisNames::PitchBack, R2 + NegativeModifier},
            {AxisNames::Twist, R0},
            {AxisNames::TwistClockwise, R0 + PositiveModifier},
            {AxisNames::TwistCounterClockwise, R0 + NegativeModifier},
            {AxisNames::Surge, L1},
            {AxisNames::SurgeForward, L1 + PositiveModifier},
            {AxisNames::SurgeBack, L1 + NegativeModifier},
            {AxisNames::Sway, L2},
            {AxisNames::SwayLeft, L2 + PositiveModifier},
            {AxisNames::SwayRight, L2 + NegativeModifier},
            {AxisNames::Vib, V0},
            {AxisNames::Suck, A0},
            {AxisNames::SuckMore, A0 + NegativeModifier},
            {AxisNames::SuckLess, A0 + PositiveModifier},
            {AxisNames::SuckPosition, A1},
            {AxisNames::SuckMorePosition, A1 + NegativeModifier},
            {AxisNames::SuckLessPosition, A1 + PositiveModifier},
            {AxisNames::Lube, A2}
        }
    }
};
