
#include "xdoublespinbox.h"

XDoubleSpinBox::XDoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox{parent}
{
    setFocusPolicy( Qt::StrongFocus );
}

void XDoubleSpinBox::wheelEvent(QWheelEvent *e)
{
    if(hasFocus())
        QDoubleSpinBox::wheelEvent(e);
}
