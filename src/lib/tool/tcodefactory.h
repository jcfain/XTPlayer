#ifndef TCODEFACTORY_H
#define TCODEFACTORY_H
#include <QString>
#include <QHash>
#include "../struct/ChannelModel.h"
#include "../handler/settingshandler.h"
#include "../lookup/AxisNames.h"
#include "boolinq.h"
#include "xmath.h"

class TCodeFactory
{
public:
    TCodeFactory(double inputStart, double inputEnd);
    void init();
    void calculate(QString axisName, double value, QVector<ChannelValueModel> &axisValues);
    QString formatTCode(QVector<ChannelValueModel>* values);

private:
    int calculateTcodeRange(double value, QString channel);
    int calculateGamepadSpeed(double gpIn);

    QHash<QString, double>* _addedAxis = new QHash<QString, double>();
    double _input_start = 0.0;
    double _input_end = 1.0;
};

#endif // TCODEFACTORY_H
