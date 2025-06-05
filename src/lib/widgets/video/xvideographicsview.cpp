#include "xvideographicsview.h"
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QGraphicsGridLayout>

XVideoGraphicsView::XVideoGraphicsView(QMediaPlayer* mediaplayer, QWidget* parent) : QGraphicsView(parent)
{
    setStyleSheet("* {background: green}");
    // setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // setAlignment(Qt::AlignLeft | Qt::AlignTop);
    // setResizeAnchor(QGraphicsView::ViewportAnchor::AnchorViewCenter);
    // setSizePolicy(QSizePolicy::ExpandFlag);
    QGraphicsScene* scene = new QGraphicsScene();
    setScene(scene);
    // scene->setSceneRect(rect());
    // scene->setBackgroundBrush(Qt::blue);
    QGraphicsVideoItem* video = new QGraphicsVideoItem();
    // video->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
    // video->setSize({scene->width(), scene->height()});
    // video->setParent(parent);
    mediaplayer->setVideoOutput(video);
    // scene->setSceneRect(parent->rect());
    // setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    scene->addItem(video);
    // fitInView(parent->rect(), Qt::KeepAspectRatio);
    // setSceneRect(parent->rect());
    // raise();
    show();
}

void XVideoGraphicsView::mouseDoubleClickEvent(QMouseEvent * e)
{
    emit doubleClicked(e);
}
void XVideoGraphicsView::mousePressEvent(QMouseEvent * e)
{
    emit singleClicked(e);
}
void XVideoGraphicsView::keyPressEvent(QKeyEvent * e)
{
    emit keyPressed(e);
}
void XVideoGraphicsView::keyReleaseEvent(QKeyEvent * e)
{
    emit keyReleased(e);
}
void XVideoGraphicsView::enterEvent(QEnterEvent * e)
{
    emit mouseEnter(e);
}

void XVideoGraphicsView::mouseMoveEvent( QMouseEvent* e ) {
    // LogHandler::Debug("Enter mouseMove: "+QString::number(e->pos().x()));
    emit mouseMove(e);
    QWidget::mouseMoveEvent(e);
}

void XVideoGraphicsView::paintEvent(QPaintEvent *e)
{
    QGraphicsView::paintEvent(e);
    // QPainter painter(this->viewport());
    // painter.
}
