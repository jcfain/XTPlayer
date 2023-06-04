
#include "xwidget.h"

XWidget::XWidget(QWidget *parent)
    : QWidget{parent}
{
    QWidget::setMouseTracking(true);
}

void XWidget::mouseMoveEvent( QMouseEvent* e ) {
    emit mouseMove(e);
}
