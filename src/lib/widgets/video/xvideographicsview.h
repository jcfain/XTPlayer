#ifndef XVIDEOGRAPHICSVIEW_H
#define XVIDEOGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>

// Renders video into a QGraphicsScene (no native window), so ordinary child
// widgets can be laid out on top of the video (controls, library list,
// subtitles, ...) with semi-transparent styling - like the web player.
class XVideoGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit XVideoGraphicsView(QWidget* parent = nullptr);
    void setPlayer(QMediaPlayer* player);
    // Letterbox-aware video rectangle in viewport coordinates
    QRect videoRect() const;
    // The internal video sink (exposes subtitle text on Qt 6)
    QVideoSink* videoSink() const { return m_videoItem->videoSink(); }

signals:
    void doubleClicked(QMouseEvent* e);
    void singleClicked(QMouseEvent* e);
    void keyPressed(QKeyEvent* k);
    void keyReleased(QKeyEvent* k);
    void mouseEnter(QEnterEvent* e);
    void mouseMove(QMouseEvent* e);

protected:
    void mouseDoubleClickEvent(QMouseEvent * e) override;
    void mousePressEvent(QMouseEvent * e) override;
    void keyPressEvent(QKeyEvent * e) override;
    void keyReleaseEvent(QKeyEvent * e) override;
    void enterEvent(QEnterEvent * e) override;
    void mouseMoveEvent( QMouseEvent* e ) override;
    void resizeEvent(QResizeEvent * e) override;

private:
    void updateVideoFit();
    QGraphicsScene* m_scene;
    QGraphicsVideoItem* m_videoItem;
};

#endif // XVIDEOGRAPHICSVIEW_H
