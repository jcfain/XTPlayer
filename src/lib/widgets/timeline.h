#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QFuture>

class TimeLine : public QWidget
{
    Q_OBJECT
public:
    explicit TimeLine(QWidget *parent = nullptr);
    ~TimeLine();

    //QSize minimumSizeHint() const override;

//    qint64 GetMinimum() const;
//    void SetMinimum(qint64 aMinimum);

    qint64 GetMinimumRange();
//    void SetMinimumRange(qint64 minimumRange);

//    qint64 GetMaximum() const;
//    void SetMaximum(qint64 aMaximum);

    qint64 getStartLoop() const;
    void setStartLoop(qint64 startLoop);

    qint64 getEndLoop() const;
    void setEndLoop(qint64 endLoop);

    bool getMousePressed();

    void setName(QString name);

    void setDuration(qint64 duration);

    qint64 getCurrentTime() const;
    void setCurrentTime(qint64 duration);

    void setLoop(bool enabled);
    void setLoopRange(qint64 loopStart, qint64 loopEnd);
    void clear();
    void togglePause(bool isPaused);
    void stop();
    void setPaintBackground(bool isPainted);

protected:
    void paintEvent(QPaintEvent* aEvent) override;
    void mousePressEvent(QMouseEvent* aEvent) override;
    void mouseMoveEvent(QMouseEvent* aEvent) override;
    void mouseReleaseEvent(QMouseEvent* aEvent) override;
    void changeEvent(QEvent* aEvent) override;
    void enterEvent(QEnterEvent * event) override;
    void leaveEvent(QEvent * event) override;

    QRectF firstHandleRect() const;
    QRectF currentHandleRect() const;
    QRectF secondHandleRect() const;

    QRectF handleRect(qint64 aValue, int width) const;
    int getTimeToWidth(qint64 time) const;
    qint64 getPositionToTime(int position) const;

signals:
    void startLoopChanged(qint64 startLoop);
    void endLoopChanged(qint64 endLoop);
    void startLoopMove(qint64 startLoop);
    void endLoopMove(qint64 endLoop);
    void loopChanged(qint64 min, qint64 max);
    void currentTimeChanged(qint64 time);
    void onHover(QPoint position, qint64 sliderValue);
    void onLeave();
    void mouseRelease(QString name);
    void mousePress();
    void updatePaint();

private:
    Q_DISABLE_COPY(TimeLine)
    QString _name;
    float currentPercentage();
    //int validLength() const;
    void syncTime();
    bool isLoop = false;
    bool m_isPaused = false;
    bool m_isBackgroundPainted = false;
    bool m_stopping = false;
//    qint64 mMinimum = 0;
//    qint64 mMaximum = 0;
    QFuture<void> m_syncTimeFuture;
    bool m_syncTimeRunning;
    qint64 m_minimumRange = 10000;
    qint64 m_startLoop = 0;
    qint64 m_currentTime = 0;
    qint64 m_endLoop = 0;
    qint64 m_duration = 0;
    bool m_startLoopPressed = false;
    bool m_endLoopPressed = false;
    bool m_currentTimePressed = false;
    int mDelta;
    int m_HandleWidth = 6;
    int m_LoopHandleWidth = 10;
    QPen m_positionPen = QPen(Qt::GlobalColor::cyan, 2, Qt::SolidLine);
    QBrush m_positionBrush = QBrush(Qt::GlobalColor::cyan);
    QPen m_positionOutlinePen = QPen(Qt::GlobalColor::black, 1, Qt::SolidLine);
};

#endif // TIMELINE_H
