#include "xvideowidget.h"

XVideoWidget::XVideoWidget(QWidget* parent) : QVideoWidget(parent)
{
    setStyleSheet("* {background: black}");
}

void XVideoWidget::mouseDoubleClickEvent(QMouseEvent * e)
{
    emit doubleClicked(e);
}
void XVideoWidget::mousePressEvent(QMouseEvent * e)
{
    emit singleClicked(e);
}
void XVideoWidget::keyPressEvent(QKeyEvent * e)
{
    emit keyPressed(e);
}
void XVideoWidget::keyReleaseEvent(QKeyEvent * e)
{
    emit keyReleased(e);
}
void XVideoWidget::enterEvent(QEvent * e)
{
    emit mouseEnter(e);
}
