#ifndef AXISNAMES_H
#define AXISNAMES_H
#include <QString>
#include <QMap>

enum class AxisNames
{
    None,
    Stroke,
    StrokeUp,
    StrokeDown,
    Sway,
    SwayLeft,
    SwayRight,
    Surge,
    SurgeBack,
    SurgeForward,
    Twist,
    TwistClockwise,
    TwistCounterClockwise,
    Roll,
    RollLeft,
    RollRight,
    Pitch,
    PitchForward,
    PitchBack,
    Vib,
    Lube,
    Suck,
    SuckPosition,
    SuctionLevel,
    AXIS_NAMES_LENGTH
};

enum class AxisType
{
    None, // 0
    Range, // 1
    Switch, // 2
    HalfRange // 3
};

const QMap<QString, AxisType> AxisTypes =
{
    {"None", AxisType::None},
    {"Range", AxisType::Range},
    {"Switch", AxisType::Switch},
    {"Half range", AxisType::HalfRange}
};

enum class AxisDimension
{
    None, // 0
    Heave, // 1
    Surge, // 2
    Sway, // 3
    Pitch, // 4
    Roll, // 5
    Yaw // 6
};

const QMap<QString, AxisDimension> AxisDimensions =
{
    {"None", AxisDimension::None},
    {"Heave", AxisDimension::Heave},
    {"Surge", AxisDimension::Surge},
    {"Sway", AxisDimension::Sway},
    {"Pitch", AxisDimension::Pitch},
    {"Roll", AxisDimension::Roll},
    {"Yaw", AxisDimension::Yaw}
};
#endif // AXISNAMES_H
