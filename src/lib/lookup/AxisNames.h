#ifndef AXISNAMES_H
#define AXISNAMES_H
#include <QString>
struct AxisNames
{
    const QString None = "None";
    const QString TcXUpDownL0 = "L0";
    const QString TcXUpL0 = "L0+";
    const QString TcXDownL0 = "L0-";
    const QString TcYLeftRightL1 = "L1";
    const QString TcYLeftL1 = "L1+";
    const QString TcYRightL1 = "L1-";
    const QString TcXForwardBackL2 = "L2";
    const QString TcXForwardL2 = "L2+";
    const QString TcXBackL2 = "L2-";
    const QString TcVibV0 = "V0";
    const QString TcPumpV2 = "V2";
    const QString TcTwistR0 = "R0";
    const QString TcTwistCWR0 = "R0+";
    const QString TcTwistCCWR0 = "R0-";
    const QString TcYRollR1 = "R1";
    const QString TcYRollLeftR1 = "R1+";
    const QString TcYRollRightR1 = "R1-";
    const QString TcXRollR2 = "R2";
    const QString TcXRollForwardR2 = "R2+";
    const QString TcXRollBackR2 = "R2-";
    const QString PositiveModifier = "+";
    const QString NegativeModifier = "-";
};
#endif // AXISNAMES_H
