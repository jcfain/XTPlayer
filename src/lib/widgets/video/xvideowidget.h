#ifndef XVIDEOWIDGET_H
#define XVIDEOWIDGET_H

#include <QVideoWidget>
#include <QObject>

class XVideoWidget : public QVideoWidget
{
    Q_OBJECT
signals:
    void doubleClicked(QMouseEvent* e);
    void singleClicked(QMouseEvent* e);
    void keyPressed(QKeyEvent* k);
    void keyReleased(QKeyEvent* k);
    void mouseEnter(QEnterEvent* e);
public:
    XVideoWidget(QWidget* parent = nullptr);

private:
    void mouseDoubleClickEvent(QMouseEvent * e) override;
    void mousePressEvent(QMouseEvent * e) override;
    void keyPressEvent(QKeyEvent * e) override;
    void keyReleaseEvent(QKeyEvent * e) override;
    void enterEvent(QEnterEvent * e) override;
};

#endif // XVIDEOWIDGET_H
