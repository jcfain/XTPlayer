#ifndef TCODECHANNELLOOKUP_H
#define TCODECHANNELLOOKUP_H
#include <QString>
#include "AxisNames.h"
#include "../handler/settingshandler.h"

class TCodeChannelLookup
{
public:
    static QString PositiveModifier;
    static QString NegativeModifier;
    static QHash<TCodeVersion,  QMap<AxisNames,  QString>> TCodeVersionMap;
    static void setSelectedTCodeVersion(TCodeVersion version);
    static QMap<AxisNames,  QString> GetSelectedVersionMap();
    static void AddUserAxis(QString channel);
    static bool ChannelExists(QString channel);
    static QString ToString(AxisNames channel);
    static QString None();
    static QString Stroke();
    static QString StrokeUp();
    static QString StrokeDown();
    static QString Sway();
    static QString SwayLeft();
    static QString SwayRight();
    static QString Surge();
    static QString SurgeBack();
    static QString SurgeForward();
    static QString Twist();
    static QString TwistClockwise();
    static QString TwistCounterClockwise();
    static QString Roll();
    static QString RollLeft();
    static QString RollRight();
    static QString Pitch();
    static QString PitchForward();
    static QString PitchBack();
    static QString Vib();
    static QString Lube();
    static QString Suck();
    static QString SuckMore();
    static QString SuckLess();
    static QString SuckPosition();
    static QString SuckMorePosition();
    static QString SuckLessPosition();
private:
    static int _channelCount;
    static QMap<AxisNames,  QString> _selectedTCodeVersionMap;
    static QString NA;
    static QString L0;
    static QString L2;
    static QString L1;
    static QString R0;
    static QString R1;
    static QString R2;
    static QString V0;
    static QString V1;
    static QString L3;
    static QString A0;
    static QString A1;
    static QString A2;
};

#endif // TCODECHANNELLOOKUP_H
