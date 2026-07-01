
#include "xcombobox.h"
#include "lib/handler/loghandler.h"

XComboBox::XComboBox(QWidget *parent)
    : QComboBox{parent}
{
    setFocusPolicy( Qt::StrongFocus );
}

void XComboBox::wheelEvent(QWheelEvent *e)
{
    if(hasFocus())
        QComboBox::wheelEvent(e);
}
