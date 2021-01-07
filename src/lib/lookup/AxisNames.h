#ifndef AXISNAMES_H
#define AXISNAMES_H
#include <QString>
#include <QMap>
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

struct AxisNames
{
    const QString PositiveModifier = "+";
    const QString NegativeModifier = "-";
    const QString None = "None";
    const QString Stroke = "L0";
    const QString StrokeUp = "L0+";
    const QString StrokeDown = "L0-";
    const QString Sway = "L2";
    const QString SwayLeft = "L2+";
    const QString SwayRight = "L2-";
    const QString Surge = "L1";
    const QString SurgeBack = "L1+";
    const QString SurgeForward = "L1-";
    const QString Twist = "R0";
    const QString TwistClockwise = "R0+";
    const QString TwistCounterClockwise = "R0-";
    const QString Roll = "R1";
    const QString RollLeft = "R1+";
    const QString RollRight = "R1-";
    const QString Pitch = "R2";
    const QString PitchForward = "R2+";
    const QString PitchBack = "R2-";
    const QString Vib = "V0";
    const QString Lube = "V1";
    const QString Suck = "L3";
};
//enum Axis
//{
//    None,
//    TcXUpDownL0,
//    TcXUpL0,
//    TcXDownL0,
//    TcYLeftRightL1,
//    TcYLeftL1,
//    TcYRightL1,
//    TcZBackForwardL2,
//    TcZBackL2,
//    TcZForwardL2,
//    TcVibV0,
//    TcPumpV2,
//    TcTwistR0,
//    TcTwistCWR0,
//    TcTwistCCWR0,
//    TcYRollR1,
//    TcYRollLeftR1,
//    TcYRollRightR1,
//    TcXRollR2,
//    TcXRollForwardR2,
//    TcXRollBackR2
//};
//struct AxisNames
//{
//    const QString PositiveModifier = "+";
//    const QString NegativeModifier = "-";
//    QMap<Axis, const QString> map = {
//        {Axis::None, "None"},
//        {Axis::TcXUpDownL0, "L0"},
//        {Axis::TcXUpL0, "L0"+PositiveModifier},
//        {Axis::TcXDownL0, "L0"+NegativeModifier},
//        {Axis::TcYLeftRightL1, "L1"},
//        {Axis::TcYLeftL1, "L1"+PositiveModifier},
//        {Axis::TcYRightL1, "L1"+NegativeModifier},
//        {Axis::TcZBackForwardL2, "L2"},
//        {Axis::TcZBackL2, "L2"+PositiveModifier},
//        {Axis::TcZForwardL2, "L2"+NegativeModifier},
//        {Axis::TcVibV0, "V0"},
//        {Axis::TcPumpV2, "V2"},
//        {Axis::TcTwistR0, "R0"},
//        {Axis::TcTwistCWR0, "R0"+PositiveModifier},
//        {Axis::TcTwistCCWR0, "R0"+NegativeModifier},
//        {Axis::TcYRollR1, "R1"},
//        {Axis::TcYRollLeftR1, "R1"+PositiveModifier},
//        {Axis::TcYRollRightR1, "R1"+NegativeModifier},
//        {Axis::TcXRollR2, "R2"},
//        {Axis::TcXRollForwardR2, "R2"+PositiveModifier},
//        {Axis::TcXRollBackR2, "R2"+NegativeModifier}
//    };
//};
#endif // AXISNAMES_H
