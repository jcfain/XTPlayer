#ifndef PLAYERXVIDEOWIDGET_H
#define PLAYERXVIDEOWIDGET_H

#include <QVideoWidget>
#include <QPainter>

class XVideoWidget : public QVideoWidget
{
   Q_OBJECT
public:
    XVideoWidget(QWidget *parent = nullptr) : QVideoWidget(parent)
    {
        //setFixedSize(20, 20);
    }
    ~XVideoWidget()
    {
    };
signals:
    void doubleClicked(QMouseEvent * event);
protected:
    void mouseDoubleClickEvent(QMouseEvent * event) override
    {
        emit doubleClicked(event);
    }
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::green);
    }
    bool event(QEvent *event) override
    {
        return QVideoWidget::event(event);
    }
    void hideEvent(QHideEvent *event) override
    {
        QVideoWidget::hideEvent(event);
    }
    void moveEvent(QMoveEvent *event) override
    {
        QVideoWidget::moveEvent(event);
    }
    void resizeEvent(QResizeEvent *event) override
    {
        QVideoWidget::resizeEvent(event);
    }
    void showEvent(QShowEvent *event) override
    {
        QVideoWidget::showEvent(event);
    }
    QMediaObject* mediaObject() const override
    {
        return QVideoWidget::mediaObject();
    }
};

#endif // PLAYERXVIDEOWIDGET_H
