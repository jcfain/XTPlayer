
#include "xspinbox.h"
#include "lib/handler/loghandler.h"

XSpinBox::XSpinBox(QWidget *parent)
    : QSpinBox{parent}
{
    setFocusPolicy( Qt::StrongFocus );
}

void XSpinBox::wheelEvent(QWheelEvent *e)
{
    if(hasFocus())
        QSpinBox::wheelEvent(e);
}
