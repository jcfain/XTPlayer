#ifndef AXISNAMES_H
#define AXISNAMES_H
#include <QString>
#include <QMap>
enum AxisType
{
    Range,
    Switch,
    HalfRange
};

struct AxisModel
{
    AxisType type;
    QString channel;
    QString friendlyName;
    QString trackName;
};

struct AxisNames
{
    const QString PositiveModifier = "+";
    const QString NegativeModifier = "-";
    const QString None = "None";
    const QString TcYUpDownL0 = "L0";
    const QString TcXUpL0 = "L0+";
    const QString TcXDownL0 = "L0-";
    const QString TcXLeftRightL2 = "L2";
    const QString TcYLeftL1 = "L2+";
    const QString TcYRightL1 = "L2-";
    const QString TcZBackForwardL1 = "L1";
    const QString TcZBackL2 = "L1+";
    const QString TcZForwardL2 = "L1-";
    const QString TcTwistR0 = "R0";
    const QString TcTwistCWR0 = "R0+";
    const QString TcTwistCCWR0 = "R0-";
    const QString TcXRollR1 = "R1";
    const QString TcYRollLeftR1 = "R1+";
    const QString TcYRollRightR1 = "R1-";
    const QString TcYRollR2 = "R2";
    const QString TcXRollForwardR2 = "R2+";
    const QString TcXRollBackR2 = "R2-";
    const QString TcVibV0 = "V0";
    const QString TcPumpV1 = "V1";
    QList<QPair<QString, AxisModel>> BasicAxis =
    {
        { TcYUpDownL0, { AxisType::Range, TcYUpDownL0, "Stroke", "" }},//heave
        { TcXLeftRightL2, { AxisType::Range, TcXLeftRightL2, "Sway", "sway" }},//sway
        { TcZBackForwardL1, { AxisType::Range, TcZBackForwardL1, "Surge", "surge" }},//surge
        { TcYRollR2, { AxisType::Range, TcYRollR2, "Pitch", "pitch" }},
        { TcXRollR1, { AxisType::Range, TcXRollR1, "Roll", "roll" }},
        { TcTwistR0, { AxisType::Range, TcTwistR0, "Twist", "twist" }},//yaw
        { TcVibV0, { AxisType::Switch, TcVibV0, "Vib", "vib" }},
        { TcPumpV1, { AxisType::Switch, TcPumpV1, "Pump", "pump" }}
    };
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
