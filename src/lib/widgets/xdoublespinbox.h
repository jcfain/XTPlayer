
#ifndef XDOUBLESPINBOX_H
#define XDOUBLESPINBOX_H


#include <QDoubleSpinBox>
#include <QMouseEvent>


class XDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit XDoubleSpinBox(QWidget *parent = nullptr);


private:
    void wheelEvent(QWheelEvent *e) override;

};

#endif // XDOUBLESPINBOX_H
