#include "rangeslider.h"
#include <QDebug>
#include <QStyle>

namespace
{

const int scHandleSideLength = 11;
const int scSliderBarHeight = 5;
const int scLeftRightMargin = 1;

}

RangeSlider::RangeSlider(QWidget* aParent)
    : QWidget(aParent),
      mMinimum(0),
      mMaximum(100),
      mLowerValue(0),
      mUpperValue(100),
      mFirstHandlePressed(false),
      mSecondHandlePressed(false),
      mInterval(mMaximum - mMinimum),
      mBackgroudColorEnabled(QColorConstants::White),
      mBackgroudColorDisabled(QColorConstants::LightGray),
      mBackgroudColor(mBackgroudColorEnabled),
      mForgroundColorEnabled(QColorConstants::Black),
      mForgroundColorDisabled(QColorConstants::LightGray),
      mForgroundColor(mForgroundColorEnabled),
      orientation(Qt::Horizontal)
{
    setMouseTracking(true);
}

RangeSlider::RangeSlider(Qt::Orientation ori, Options t, QWidget* aParent)
    : QWidget(aParent),
      mMinimum(0),
      mMaximum(100),
      mLowerValue(0),
      mUpperValue(100),
      mFirstHandlePressed(false),
      mSecondHandlePressed(false),
      mInterval(mMaximum - mMinimum),
      mBackgroudColorEnabled(QColorConstants::White),
      mBackgroudColorDisabled(QColorConstants::LightGray),
      mBackgroudColor(mBackgroudColorEnabled),
      mForgroundColorEnabled(QColorConstants::Black),
      mForgroundColorDisabled(QColorConstants::LightGray),
      mForgroundColor(mForgroundColorEnabled),
      orientation(ori),
      type(t)
{
    setMouseTracking(true);
}

void RangeSlider::paintEvent(QPaintEvent* aEvent)
{
    Q_UNUSED(aEvent);
    QPainter painter(this);

    // Background
    QRectF backgroundRect;
    if(orientation == Qt::Horizontal)
        backgroundRect = QRectF(scLeftRightMargin, (height() - scSliderBarHeight) / 2, width() - scLeftRightMargin * 2, scSliderBarHeight);
    else
        backgroundRect = QRectF((width() - scSliderBarHeight) / 2, scLeftRightMargin, scSliderBarHeight, height() - scLeftRightMargin*2);

    QPen pen(Qt::gray, 0.8);
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Qt4CompatiblePainting);
    QBrush backgroundBrush(mBackgroudColor);
    painter.setBrush(backgroundBrush);
    painter.drawRoundedRect(backgroundRect, 1, 1);

    // First value handle rect
    pen.setColor(Qt::darkGray);
    pen.setWidth(0.5);
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing);
    QBrush handleBrush(mBackgroudColor);
    painter.setBrush(handleBrush);
    QRectF leftHandleRect = firstHandleRect();
    if(type.testFlag(LeftHandle))
        painter.drawRoundedRect(leftHandleRect, 4, 4);

    // Second value handle rect
    QRectF rightHandleRect = secondHandleRect();
    if(type.testFlag(RightHandle))
        painter.drawRoundedRect(rightHandleRect, 4, 4);

    // Handles
    painter.setRenderHint(QPainter::Antialiasing, false);
    QRectF selectedRect(backgroundRect);
    if(orientation == Qt::Horizontal) {
        selectedRect.setLeft((type.testFlag(LeftHandle) ? leftHandleRect.right() : leftHandleRect.left()) + 0.5);
        selectedRect.setRight((type.testFlag(RightHandle) ? rightHandleRect.left() : rightHandleRect.right()) - 0.5);
    } else {
        selectedRect.setTop((type.testFlag(LeftHandle) ? leftHandleRect.bottom() : leftHandleRect.top()) + 0.5);
        selectedRect.setBottom((type.testFlag(RightHandle) ? rightHandleRect.top() : rightHandleRect.bottom()) - 0.5);
    }
    QBrush selectedBrush(mForgroundColor);
    painter.setBrush(selectedBrush);
    painter.drawRect(selectedRect);
}

QRectF RangeSlider::firstHandleRect() const
{
    float percentage = (mLowerValue - mMinimum) * 1.0 / mInterval;
    return handleRect(percentage * validLength() + scLeftRightMargin);
}

QRectF RangeSlider::secondHandleRect() const
{
    float percentage = (mUpperValue - mMinimum) * 1.0 / mInterval;
    return handleRect(percentage * validLength() + scLeftRightMargin + (type.testFlag(LeftHandle) ? scHandleSideLength : 0));
}

QRectF RangeSlider::handleRect(int aValue) const
{
    if(orientation == Qt::Horizontal)
        return QRect(aValue, (height()-scHandleSideLength) / 2, scHandleSideLength, scHandleSideLength);
    else
        return QRect((width()-scHandleSideLength) / 2, aValue, scHandleSideLength, scHandleSideLength);
}

void RangeSlider::mousePressEvent(QMouseEvent* aEvent)
{
    if (!isEnabled())
    {
        return;
    }
    if(aEvent->buttons() & Qt::LeftButton)
    {
        int posCheck, posMax, posValue, sliderLength, firstHandleRectPosValue, secondHandleRectPosValue;
        posCheck = (orientation == Qt::Horizontal) ? aEvent->pos().y() : aEvent->pos().x();
        posMax = (orientation == Qt::Horizontal) ? height() : width();
        sliderLength = (orientation == Qt::Horizontal) ?  width() :height();
        posValue = (orientation == Qt::Horizontal) ? aEvent->pos().x() : aEvent->pos().y();
        firstHandleRectPosValue = (orientation == Qt::Horizontal) ? firstHandleRect().x() : firstHandleRect().y();
        secondHandleRectPosValue = (orientation == Qt::Horizontal) ? secondHandleRect().x() : secondHandleRect().y();

        mSecondHandlePressed = secondHandleRect().contains(aEvent->pos());
        mFirstHandlePressed = !mSecondHandlePressed && firstHandleRect().contains(aEvent->pos());
        if(mFirstHandlePressed)
        {
            mDelta = posValue - (firstHandleRectPosValue + scHandleSideLength / 2);
        }
        else if(mSecondHandlePressed)
        {
            mDelta = posValue - (secondHandleRectPosValue + scHandleSideLength / 2);
        }

        if(posCheck >= 2
           && posCheck <= posMax - 2)
        {
            int destinationValue = XMath::mapRange(posValue, 0, sliderLength, GetMinimum(), GetMaximum());
            //Click outside of range below first handle
            if(posValue < firstHandleRectPosValue)
            {
                setLowerValue(destinationValue);
                emit lowerValueMove(destinationValue);
            }

            //Click inside of range
            else if(((posValue > firstHandleRectPosValue + scHandleSideLength) || !type.testFlag(LeftHandle))
                    && ((posValue < secondHandleRectPosValue) || !type.testFlag(RightHandle)))
            {
                if(type.testFlag(DoubleHandles))
                {
                    if(posValue - (firstHandleRectPosValue + scHandleSideLength) <
                            (secondHandleRectPosValue - (firstHandleRectPosValue + scHandleSideLength)) / 2)
                    {
                        //int value = (mLowerValue + slidervalue < mUpperValue) ? mLowerValue + slidervalue : mUpperValue;
                        int value = (destinationValue > mUpperValue - _aMinimumRange) ? mUpperValue - _aMinimumRange : destinationValue;
                        setLowerValue(value);
                        emit lowerValueMove(value);
                    }
                    else
                    {
                        //int value = (mUpperValue - slidervalue > mLowerValue) ? mUpperValue - slidervalue : mLowerValue;
                        int value = (destinationValue < mLowerValue + _aMinimumRange) ? mLowerValue + _aMinimumRange : destinationValue;
                        setUpperValue(value);
                        emit upperValueMove(value);
                    }
                }
                else if(type.testFlag(LeftHandle))
                {
                    //int value = (mLowerValue + slidervalue < mUpperValue) ? mLowerValue + slidervalue : mUpperValue;
                    setLowerValue(destinationValue);
                    emit lowerValueMove(destinationValue);
                }
                else if(type.testFlag(RightHandle))
                {
                    //int value = (mUpperValue - slidervalue > mLowerValue) ? mUpperValue - slidervalue : mLowerValue;
                    setUpperValue(destinationValue);
                    emit upperValueMove(destinationValue);
                }
            }
            //Click outside of range beyond second handle
            else if(posValue > secondHandleRectPosValue + scHandleSideLength)
            {
                setUpperValue(destinationValue);
                emit upperValueMove(destinationValue);
            }
        }
    }
}

void RangeSlider::mouseMoveEvent(QMouseEvent* aEvent)
{
    if (!isEnabled())
    {
        return;
    }
    if(aEvent->buttons() & Qt::LeftButton)
    {
        int posValue, firstHandleRectPosValue, secondHandleRectPosValue;
        posValue = (orientation == Qt::Horizontal) ? aEvent->pos().x() : aEvent->pos().y();
        firstHandleRectPosValue = (orientation == Qt::Horizontal) ? firstHandleRect().x() : firstHandleRect().y();
        secondHandleRectPosValue = (orientation == Qt::Horizontal) ? secondHandleRect().x() : secondHandleRect().y();

        if(mFirstHandlePressed && type.testFlag(LeftHandle))
        {
            if(posValue - mDelta + scHandleSideLength / 2 <= secondHandleRectPosValue)
            {
                int lowerValue = (posValue - mDelta - scLeftRightMargin - scHandleSideLength / 2) * 1.0 / validLength() * mInterval + mMinimum;
                setLowerValue(lowerValue);
                emit lowerValueMove(lowerValue);
            }
            else
            {
                setLowerValue(mUpperValue - _aMinimumRange);
                emit lowerValueMove(mUpperValue -_aMinimumRange);
            }
        }
        else if(mSecondHandlePressed && type.testFlag(RightHandle))
        {
            if(firstHandleRectPosValue + scHandleSideLength * (type.testFlag(DoubleHandles) ? 1.5 : 0.5) <= posValue - mDelta)
            {
                int upperValue = (posValue - mDelta - scLeftRightMargin - scHandleSideLength / 2 - (type.testFlag(DoubleHandles) ? scHandleSideLength : 0)) * 1.0 / validLength() * mInterval + mMinimum;
                setUpperValue(upperValue);
                emit upperValueMove(upperValue);
            }
            else
            {
                setUpperValue(mLowerValue + _aMinimumRange);
                emit upperValueMove(mLowerValue +_aMinimumRange);
            }
        }
    }


    const int o = style()->pixelMetric(QStyle::PM_SliderLength ) - 1;
    int sliderValue = QStyle::sliderValueFromPosition(GetMinimum(), GetMaximum(), aEvent->pos().x()-o/2, width()-o, false);
    emit onHover(aEvent->x(), sliderValue);
}

void RangeSlider::enterEvent(QEvent * event)
{

}

void RangeSlider::leaveEvent(QEvent * event)
{
    emit onLeave();
}
void RangeSlider::mouseReleaseEvent(QMouseEvent* aEvent)
{
    Q_UNUSED(aEvent);

    mFirstHandlePressed = false;
    mSecondHandlePressed = false;
    emit mouseRelease(_name);
}

void RangeSlider::changeEvent(QEvent* aEvent)
{
    if(aEvent->type() == QEvent::EnabledChange)
    {
        updateColor();
    }
}

QSize RangeSlider::minimumSizeHint() const
{
    return QSize(scHandleSideLength * 2 + scLeftRightMargin * 2, scHandleSideLength);
}

void RangeSlider::setName(QString name)
{
    _name = name;
}

int RangeSlider::GetMinimum() const
{
    return mMinimum;
}

void RangeSlider::SetMinimum(int aMinimum)
{
    setMinimum(aMinimum);
}

int RangeSlider::GetMaximum() const
{
    return mMaximum;
}

void RangeSlider::SetMaximum(int aMaximum)
{
    setMaximum(aMaximum);
}

int RangeSlider::GetLowerValue() const
{
    return mLowerValue;
}

void RangeSlider::SetLowerValue(int aLowerValue)
{
    setLowerValue(aLowerValue);
}

int RangeSlider::GetUpperValue() const
{
    return mUpperValue;
}

void RangeSlider::SetUpperValue(int aUpperValue)
{
    setUpperValue(aUpperValue);
}

void RangeSlider::setLowerValue(int aLowerValue)
{
    if(aLowerValue > mMaximum)
    {
        aLowerValue = mMaximum;
    }

    if(aLowerValue < mMinimum)
    {
        aLowerValue = mMinimum;
    }

    mLowerValue = aLowerValue;
    emit lowerValueChanged(_name, mLowerValue);
    emit lowerValueChanged(mLowerValue);

    update();
}

void RangeSlider::setUpperValue(int aUpperValue)
{
    if(aUpperValue > mMaximum)
    {
        aUpperValue = mMaximum;
    }

    if(aUpperValue < mMinimum)
    {
        aUpperValue = mMinimum;
    }

    mUpperValue = aUpperValue;
    emit upperValueChanged(_name, mUpperValue);
    emit upperValueChanged(mUpperValue);

    update();
}

void RangeSlider::setMinimum(int aMinimum)
{
    if(aMinimum <= mMaximum)
    {
        mMinimum = aMinimum;
    }
    else
    {
        int oldMax = mMaximum;
        mMinimum = oldMax;
        mMaximum = aMinimum;
    }
    mInterval = mMaximum - mMinimum;
    update();

    setLowerValue(mMinimum);
    setUpperValue(mMaximum);

    emit rangeChanged(mMinimum, mMaximum);
}

void RangeSlider::setMaximum(int aMaximum)
{
    if(aMaximum >= mMinimum)
    {
        mMaximum = aMaximum;
    }
    else
    {
        int oldMin = mMinimum;
        mMaximum = oldMin;
        mMinimum = aMaximum;
    }
    mInterval = mMaximum - mMinimum;
    update();

    setLowerValue(mMinimum);
    setUpperValue(mMaximum);

    emit rangeChanged(mMinimum, mMaximum);
}

int RangeSlider::validLength() const
{
    int len = (orientation == Qt::Horizontal) ? width() : height();
    return len - scLeftRightMargin * 2 - scHandleSideLength * (type.testFlag(DoubleHandles) ? 2 : 1);
}

void RangeSlider::SetRange(int aMinimum, int mMaximum)
{
    setMinimum(aMinimum);
    setMaximum(mMaximum);
}
int RangeSlider::GetMinimumRange()
{
    return _aMinimumRange;
}
void RangeSlider::SetMinimumRange(int aMinimumRange)
{
    _aMinimumRange = aMinimumRange;
}
void RangeSlider::setOrientation(Qt::Orientation ori)
{
    orientation = ori;
}

void RangeSlider::setOption(Options t)
{
    type = t;
}

void RangeSlider::setBackGroundEnabledColor(QColor color)
{
    mBackgroudColorEnabled = color;
    updateColor();
}

void RangeSlider::setForgroundEnabledColor(QColor color)
{
    mForgroundColorEnabled = color;
    updateColor();
}

void RangeSlider::setBackGroundDisabledColor(QColor color)
{
    mBackgroudColorDisabled = color;
    updateColor();
}
void RangeSlider::setForgroundDisabledColor(QColor color)
{
    mForgroundColorDisabled = color;
    updateColor();
}

 void RangeSlider::updateColor()
 {
     if(isEnabled())
     {
         mBackgroudColor = mBackgroudColorEnabled;
         mForgroundColor = mForgroundColorEnabled;
     }
     else
     {
         mBackgroudColor = mBackgroudColorDisabled;
         mForgroundColor = mForgroundColorDisabled;
     }
     update();
 }
