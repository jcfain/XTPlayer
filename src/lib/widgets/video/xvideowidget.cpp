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
void XVideoWidget::enterEvent(QEnterEvent * e)
{
    emit mouseEnter(e);
}

void XVideoWidget::mouseMoveEvent( QMouseEvent* e ) {
    // LogHandler::Debug("Enter mouseMove: "+QString::number(e->pos().x()));
    emit mouseMove(e);
    QWidget::mouseMoveEvent(e);
}
