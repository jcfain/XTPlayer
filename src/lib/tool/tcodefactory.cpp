#include "tcodefactory.h"

TCodeFactory::TCodeFactory(double inputStart, double inputEnd)
{
    _input_start = inputStart;
    _input_end = inputEnd;
}
void TCodeFactory::init()
{
    _addedAxis->clear();
}

void TCodeFactory::calculate(QString axisName, double value, QVector<ChannelValueModel> &axisValues)
{
    AxisNames axisNames;
    auto tcodeAxisName = SettingsHandler::getGamePadButtonMap(axisName);
    if (tcodeAxisName != axisNames.None)
    {
        auto tcodeAxis = SettingsHandler::getAvailableAxis(tcodeAxisName);
        auto isNegative = tcodeAxis.AxisName.contains(axisNames.NegativeModifier);
        //auto isPositive = tcodeAxis.AxisName.contains(axisNames.PositiveModifier);
        if (_addedAxis->contains(tcodeAxis.Channel) && _addedAxis->value(tcodeAxis.Channel) == 0 && value != 0)
        {
            _addedAxis->remove(tcodeAxis.Channel);
            const ChannelValueModel cvm = boolinq::from(axisValues).firstOrDefault([tcodeAxis](const ChannelValueModel &x) { return x.Channel == tcodeAxis.Channel; });
            axisValues.removeOne(cvm);
        }
        if (!boolinq::from(axisValues).any([tcodeAxis](const ChannelValueModel &x) { return x.Channel == tcodeAxis.Channel; }))
        {
            double calculatedValue = value;
            if (isNegative && value > 0)
            {
                calculatedValue = -(value);
            }
            if (value != 0 &&
                (((tcodeAxis.AxisName == axisNames.TcXUpDownL0 || tcodeAxis.AxisName == axisNames.TcXUpL0 || tcodeAxis.AxisName == axisNames.TcXDownL0) && SettingsHandler::getInverseTcXL0()) ||
                ((tcodeAxis.AxisName == axisNames.TcXRollR2 || tcodeAxis.AxisName == axisNames.TcXRollForwardR2 || tcodeAxis.AxisName == axisNames.TcXRollBackR2) && SettingsHandler::getInverseTcXRollR2()) ||
                ((tcodeAxis.AxisName == axisNames.TcYRollR1 || tcodeAxis.AxisName == axisNames.TcYRollLeftR1 || tcodeAxis.AxisName == axisNames.TcYRollRightR1) && SettingsHandler::getInverseTcYRollR1())))
            {
                calculatedValue = -(value);
            }
            axisValues.append({
                calculateTcodeRange(calculatedValue, tcodeAxis.Channel),
                tcodeAxis.Channel
            });
            _addedAxis->insert(tcodeAxis.Channel, value);
        }
    }
}

QString TCodeFactory::formatTCode(QVector<ChannelValueModel>* values)
{
    QString tCode = "";
    foreach (auto value, *values)
    {
        auto minValue = SettingsHandler::getAvailableAxis(value.Channel).Min;
        auto maxValue = SettingsHandler::getAvailableAxis(value.Channel).Max;
        auto clampedValue = maxValue == 0 ? value.Value : XMath::constrain(value.Value, minValue, maxValue);
        tCode += value.Channel + (clampedValue < 10 ? "0" : "") + QString::number(clampedValue) + "S1000 ";
    }
    return tCode.trimmed();
}

int TCodeFactory::calculateTcodeRange(double value, QString channel)
{
    auto output_end = SettingsHandler::getAvailableAxis(channel).Max;
    auto output_start = SettingsHandler::getAvailableAxis(channel).Mid;
    auto slope = (output_end - output_start) / (_input_end - _input_start);
    return (int)(output_start + slope * (value - _input_start));
}

int TCodeFactory::calculateGamepadSpeed(double gpIn)
{
    //return (int)(gpIn < 0 ? -gpIn * SettingsHandler::getSpeed : gpIn * SettingsHandler::getSpeed);
}
