#ifndef XVIDEOGRAPHICSVIEW_H
#define XVIDEOGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMediaPlayer>

class XVideoGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    XVideoGraphicsView(QMediaPlayer* mediaplayer, QWidget* parent = nullptr);

signals:
    void doubleClicked(QMouseEvent* e);
    void singleClicked(QMouseEvent* e);
    void keyPressed(QKeyEvent* k);
    void keyReleased(QKeyEvent* k);
    void mouseEnter(QEnterEvent* e);
    void mouseMove(QMouseEvent* e);

private:
    void mouseDoubleClickEvent(QMouseEvent * e) override;
    void mousePressEvent(QMouseEvent * e) override;
    void keyPressEvent(QKeyEvent * e) override;
    void keyReleaseEvent(QKeyEvent * e) override;
    void enterEvent(QEnterEvent * e) override;
    void mouseMoveEvent( QMouseEvent* e ) override;
    void paintEvent(QPaintEvent *e) override;
};

#endif // XVIDEOGRAPHICSVIEW_H
