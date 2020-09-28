#pragma once

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include "lib/tool/xmath.h"

class RangeSlider : public QWidget
{
    Q_OBJECT
    Q_ENUMS(RangeSliderTypes)

public:
    enum Option {
        NoHandle = 0x0,
        LeftHandle = 0x1,
        RightHandle = 0x2,
        DoubleHandles = LeftHandle | RightHandle
    };
    Q_DECLARE_FLAGS(Options, Option)

    RangeSlider( QWidget* aParent = Q_NULLPTR);
    RangeSlider( Qt::Orientation ori, Options t = DoubleHandles, QWidget* aParent = Q_NULLPTR);

    QSize minimumSizeHint() const override;

    int GetMinimum() const;
    void SetMinimum(int aMinimum);

    int GetMinimumRange();
    void SetMinimumRange(int aMinimumRange);

    int GetMaximum() const;
    void SetMaximum(int aMaximum);

    int GetLowerValue() const;
    void SetLowerValue(int aLowerValue);

    int GetUpperValue() const;
    void SetUpperValue(int aUpperValue);

    void SetRange(int aMinimum, int aMaximum);
    void setOrientation(Qt::Orientation ori);
    void setOption(Options t = DoubleHandles);
    void setBackGroundEnabledColor(QColor color);
    void setForgroundEnabledColor(QColor color);
    void setBackGroundDisabledColor(QColor color);
    void setForgroundDisabledColor(QColor color);
    void updateColor();

protected:
    void paintEvent(QPaintEvent* aEvent) override;
    void mousePressEvent(QMouseEvent* aEvent) override;
    void mouseMoveEvent(QMouseEvent* aEvent) override;
    void mouseReleaseEvent(QMouseEvent* aEvent) override;
    void changeEvent(QEvent* aEvent) override;
    void enterEvent(QEvent * event) override;
    void leaveEvent(QEvent * event) override;

    QRectF firstHandleRect() const;
    QRectF secondHandleRect() const;
    QRectF handleRect(int aValue) const;

signals:
    void lowerValueChanged(int aLowerValue);
    void upperValueChanged(int aUpperValue);
    void lowerValueMove(int aLowerValue);
    void upperValueMove(int aUpperValue);
    void rangeChanged(int aMin, int aMax);
    void onHover(int position, int sliderValue);
    void onLeave();

public slots:
    void setLowerValue(int aLowerValue);
    void setUpperValue(int aUpperValue);
    void setMinimum(int aMinimum);
    void setMaximum(int aMaximum);

private:
    Q_DISABLE_COPY(RangeSlider)
    float currentPercentage();
    int validLength() const;

    int mMinimum;
    int mMaximum;
    int _aMinimumRange = 1;
    int mLowerValue;
    int mUpperValue;
    bool mFirstHandlePressed;
    bool mSecondHandlePressed;
    int mInterval;
    int mDelta;

    QColor mBackgroudColorEnabled;
    QColor mBackgroudColorDisabled;
    QColor mBackgroudColor;

    QColor mForgroundColorEnabled;
    QColor mForgroundColorDisabled;
    QColor mForgroundColor;

    Qt::Orientation orientation;
    Options type;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(RangeSlider::Options)
