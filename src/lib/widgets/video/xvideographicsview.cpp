#include "xvideographicsview.h"
#include <QVideoSink>

XVideoGraphicsView::XVideoGraphicsView(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_videoItem(new QGraphicsVideoItem())
{
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignCenter);
    setMouseTracking(true);
    setDragMode(QGraphicsView::NoDrag);
    setRenderHint(QPainter::Antialiasing, false);
    setStyleSheet("XVideoGraphicsView {background: black;}");

    m_scene->setBackgroundBrush(QBrush(Qt::black));
    m_videoItem->setAspectRatioMode(Qt::KeepAspectRatio);
    m_scene->addItem(m_videoItem);
    setScene(m_scene);

    // Keep the scene rect and view transform in sync with the video size.
    // Triggered by the item's size-change signal and by the sink's size
    // change (set when the player starts rendering) - whichever reports the
    // size first wins, and resizeEvent re-fits after any view resize.
#if BUILD_QT6
    connect(m_videoItem, &QGraphicsVideoItem::nativeSizeChanged, this, &XVideoGraphicsView::updateVideoFit);
    if(m_videoItem->videoSink())
        connect(m_videoItem->videoSink(), &QVideoSink::videoSizeChanged, this, &XVideoGraphicsView::updateVideoFit);
#else
    connect(m_videoItem, &QGraphicsVideoItem::videoSizeChanged, this, &XVideoGraphicsView::updateVideoFit);
    if(m_videoItem->videoSink())
        connect(m_videoItem->videoSink(), &QVideoSink::videoSurfaceFormatChanged, this, &XVideoGraphicsView::updateVideoFit);
#endif
}

void XVideoGraphicsView::setPlayer(QMediaPlayer* player) {
    if(player)
        player->setVideoOutput(m_videoItem);
}

void XVideoGraphicsView::updateVideoFit() {
    QSizeF size;
#if BUILD_QT6
    size = m_videoItem->nativeSize();
    if((size.width() <= 0 || size.height() <= 0) && m_videoItem->videoSink())
        size = m_videoItem->videoSink()->videoSize();
#else
    size = m_videoItem->videoSize();
    if((size.width() <= 0 || size.height() <= 0) && m_videoItem->videoSink())
        size = m_videoItem->videoSink()->videoSurfaceFormat().pixelSize();
#endif
    if(size.width() <= 0 || size.height() <= 0)
        return;
    QRectF target(0, 0, size.width(), size.height());
    if(m_scene->sceneRect() != target) {
        m_scene->setSceneRect(target);
#if BUILD_QT6
        m_videoItem->setSize(target.size());
#else
        m_videoItem->setRect(target);
#endif
    }
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

QRect XVideoGraphicsView::videoRect() const {
    if(m_scene->sceneRect().width() <= 0 || m_scene->sceneRect().height() <= 0)
        return viewport()->rect();
    return transform().mapRect(m_scene->sceneRect()).toRect().intersected(viewport()->rect());
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
    emit mouseMove(e);
    QGraphicsView::mouseMoveEvent(e);
}

void XVideoGraphicsView::resizeEvent(QResizeEvent * e) {
    QGraphicsView::resizeEvent(e);
    updateVideoFit();
}
