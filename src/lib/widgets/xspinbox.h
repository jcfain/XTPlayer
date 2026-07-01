
#ifndef XSPINBOX_H
#define XSPINBOX_H


#include <QSpinBox>
#include <QMouseEvent>


class XSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit XSpinBox(QWidget *parent = nullptr);


private:
    void wheelEvent(QWheelEvent *e) override;

};

#endif // XSPINBOX_H
