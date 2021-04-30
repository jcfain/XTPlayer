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
struct TCodeChannels
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

const QMap<QString, TCodeChannels> TCodeChannels2 =
{
    {
      "TCode v0.2",
      {
          "+",
          "-",
          "None",
          "L0",
          "L0+",
          "L0-",
          "L2",
          "L2+",
          "L2-",
          "L1",
          "L1+",
          "L1-",
          "R0",
          "R0+",
          "R0-",
          "R1",
          "R1+",
          "R1-",
          "R2",
          "R2+",
          "R2-",
          "V0",
          "V1",
          "L3"
      }
    }
};
const QMap<QString, QMap<QString, QString>> ChannelNames =
{
    {
        "TCode v0.2",
        {
            {"L0", "Stroke"},
            {"L1", "Surge"},
            {"L2", "Sway"},
            {"L3", "Suck"},
            {"R0", "Twist"},
            {"R1", "Roll"},
            {"R2", "Pitch"},
            {"V0", "Vib"},
            {"V1", "Lube"}
        }
    },
    {
        "TCode v0.3",
        {
            {"L0", "Stroke"},
            {"L1", "Surge"},
            {"L2", "Sway"},
            {"R0", "Twist"},
            {"R1", "Roll"},
            {"R2", "Pitch"},
            {"V0", "Vib"},
            {"V1", "Vib1"},
            {"A0", "Valve"},
            {"A1", "Suck"},
            {"A2", "Lube"}
        }
    }
};
#endif // AXISNAMES_H
