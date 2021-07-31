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
    if (axisName != TCodeChannelLookup::None())
    {
        ChannelModel tcodeAxis = SettingsHandler::getAxis(axisName);
        bool isNegative = tcodeAxis.AxisName.contains(TCodeChannelLookup::NegativeModifier);
        //auto isPositive = tcodeAxis.AxisName.contains(TCodeChannelLookup::PositiveModifier);
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
                (((tcodeAxis.AxisName == TCodeChannelLookup::Stroke() || tcodeAxis.AxisName == TCodeChannelLookup::StrokeUp() || tcodeAxis.AxisName == TCodeChannelLookup::StrokeDown()) && SettingsHandler::getInverseTcXL0()) ||
                ((tcodeAxis.AxisName == TCodeChannelLookup::Pitch() || tcodeAxis.AxisName == TCodeChannelLookup::PitchForward() || tcodeAxis.AxisName == TCodeChannelLookup::PitchBack()) && SettingsHandler::getInverseTcXRollR2()) ||
                ((tcodeAxis.AxisName == TCodeChannelLookup::Roll() || tcodeAxis.AxisName == TCodeChannelLookup::RollLeft() || tcodeAxis.AxisName == TCodeChannelLookup::RollRight()) && SettingsHandler::getInverseTcYRollR1())))
            {
                calculatedValue = -(value);
            }
            axisValues.append({
                calculateTcodeRange(calculatedValue, tcodeAxis),
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
        if(!value.Channel.isEmpty())
        {
            auto minValue = SettingsHandler::getAxis(value.Channel).Min;
            auto maxValue = SettingsHandler::getAxis(value.Channel).Max;
            auto clampedValue = maxValue == 0 ? value.Value : XMath::constrain(value.Value, minValue, maxValue);
            tCode += value.Channel + (clampedValue < 10 ? "0" : "") + QString::number(clampedValue) + "S" + QString::number(SettingsHandler::getLiveGamepadSpeed()) + " ";
        }
    }
    return tCode.trimmed();
}

int TCodeFactory::calculateTcodeRange(double value, ChannelModel channel)
{
    int output_end = SettingsHandler::getAxis(channel.Channel).UserMax;
    int min = SettingsHandler::getAxis(channel.Channel).UserMin;
    // Update for live x range switch
    if(channel.Channel == TCodeChannelLookup::Stroke())
    {
        output_end = SettingsHandler::getLiveXRangeMax();
        min = SettingsHandler::getLiveXRangeMin();
    }
    int output_start = channel.Type != AxisType::Switch ? qRound((output_end + min) / 2.0) : min;
    double slope = (output_end - output_start) / (_input_end - _input_start);
    return qRound(output_start + slope * (value - _input_start));
}

int TCodeFactory::calculateGamepadSpeed(double gpIn)
{
    //return (int)(gpIn < 0 ? -gpIn * SettingsHandler::getSpeed : gpIn * SettingsHandler::getSpeed);
}
