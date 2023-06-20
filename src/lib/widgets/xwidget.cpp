
#include "xwidget.h"
#include "lib/handler/loghandler.h"

XWidget::XWidget(QWidget *parent)
    : QWidget{parent}
{
   //setMouseTracking(true);
   //install_filter(this, this);
}

void XWidget::mouseMoveEvent( QMouseEvent* e ) {
    LogHandler::Debug("Enter mouseMove: "+QString::number(e->pos().x()));
    emit mouseMove(e);
    QWidget::mouseMoveEvent(e);
}

//bool XWidget::eventFilter(QObject *watched, QEvent *event)
//{
//    static int c = 0;
//    qDebug() << "EVENT: " << ++c << " - " << event->type();
//    if (event->type() == QEvent::MouseMove)
//    {
//        emit mouseMove(static_cast<QMouseEvent *>(event) );
//        return false;
//    }

//    return QWidget::eventFilter(watched, event);
//}

//// Private
//void XWidget::install_filter(QObject *target, QObject *filter)
//{
//    if (target->isWidgetType() )
//     {
//         qDebug("INSTALL");
//         static_cast<QWidget *>(target)->setMouseTracking(true);
//         target->installEventFilter(filter);
//     }
//     const QObjectList &children = target->children();
//     for (auto i = children.begin(); i < children.end(); i++)
//     {
//         if(static_cast<QObject *>(*i)->isWidgetType())
//            static_cast<QWidget *>(*i)->setMouseTracking(true);
//         install_filter(*i, filter);
//     }
//}
