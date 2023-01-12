#include "timeline.h"
#include <QStyle>
#include <QPainterPath>
#include <QtConcurrent/QtConcurrent>
#include "lib/tool/xmath.h"


TimeLine::TimeLine(QWidget* parent)
    : QWidget(parent),
      m_startLoop(0),
      m_endLoop(100),
      m_startLoopPressed(false),
      m_endLoopPressed(false)
{
    setMouseTracking(true);
    setProperty("cssClass", "TimeLine");
    setMinimumSize(QSize(parent->minimumWidth(), 20));
    connect(this, &TimeLine::updatePaint, this, [this]() {
        update();
    });
}

TimeLine::~TimeLine() {
    clear();
}

void TimeLine::paintEvent(QPaintEvent* aEvent)
{
    Q_UNUSED(aEvent);
    QPainter painter(this);

    if(m_isBackgroundPainted) {
        int posHeight = qRound(height() * 0.25);
        painter.fillRect(QRectF(0, posHeight, width(), height() - posHeight * 2), Qt::GlobalColor::black);
    }
    if(m_duration > 0)
    {
        if(isLoop)
        {
            QRectF startRect = firstHandleRect();
            QPainterPath startLoop;
    //        startLoop.moveTo(startRect.left() + (startRect.width() / 2), startRect.top());
    //        startLoop.lineTo(startRect.bottomLeft());
    //        startLoop.lineTo(startRect.bottomRight());
    //        startLoop.lineTo(startRect.left() + (startRect.width() / 2), startRect.top());
            startLoop.moveTo(startRect.topLeft());
            startLoop.lineTo(startRect.bottomLeft());
            startLoop.lineTo(startRect.right(), (startRect.height() / 2));
            startLoop.lineTo(startRect.topLeft());
            painter.fillPath(startLoop, m_positionBrush);

//            painter.setPen(m_positionOutlinePen);
//            painter.drawPath(startLoop);


            QRectF endRect = secondHandleRect();
            QPainterPath endLoop;
    //        endLoop.moveTo(endRect.left() + (endRect.width() / 2), endRect.top());
    //        endLoop.lineTo(endRect.bottomLeft());
    //        endLoop.lineTo(endRect.bottomRight());
    //        endLoop.lineTo(endRect.left() + (endRect.width() / 2), endRect.top());
            endLoop.moveTo(endRect.topRight());
            endLoop.lineTo(endRect.bottomRight());
            endLoop.lineTo(endRect.left(), (endRect.height() / 2));
            endLoop.lineTo(endRect.topRight());
            painter.fillPath(endLoop, m_positionBrush);

//            painter.setPen(m_positionOutlinePen);
//            painter.drawPath(endLoop);
        }

        if(m_currentTime > 0) {
            QRectF currentRect = currentHandleRect();
            QPainterPath currentPath;
            int halfWidth = (currentRect.width() / 2);
            //T line
//            currentPath.moveTo(currentRect.topLeft());
//            currentPath.lineTo(currentRect.topRight());
//            currentPath.moveTo(currentRect.top(), m_currentTime);
//            currentPath.lineTo(currentRect.bottom(), m_currentTime);
//            currentPath.moveTo(currentRect.bottomLeft());
//            currentPath.lineTo(currentRect.bottomRight());
            // Triangle top bottom
            currentPath.moveTo(currentRect.left() + halfWidth, currentRect.top() );
            currentPath.lineTo(currentRect.topLeft());
            currentPath.lineTo(currentRect.topRight());
            currentPath.lineTo(currentRect.left() + halfWidth, currentRect.top() + m_HandleWidth);
            currentPath.lineTo(currentRect.topLeft());

            currentPath.moveTo(currentRect.left() + halfWidth, currentRect.bottom() );
            currentPath.lineTo(currentRect.bottomLeft());
            currentPath.lineTo(currentRect.bottomRight());
            currentPath.lineTo(currentRect.left() + halfWidth, currentRect.bottom() - m_HandleWidth);
            currentPath.lineTo(currentRect.bottomLeft());

            painter.fillPath(currentPath, m_positionBrush);
            painter.setPen(m_positionPen);
            currentPath.moveTo(currentRect.left() + halfWidth, currentRect.top());
            painter.drawLine(currentRect.x() + halfWidth, 0, currentRect.x() + halfWidth, height());
        }
    }
}

void TimeLine::setDuration(qint64 duration) {
    m_duration = duration;
//    if(duration > 0) {
//        clear();
//    }
    update();
}

void TimeLine::setCurrentTime(qint64 time) {
    m_currentTime = time;
    update();
//#if !defined(Q_OS_LINUX)
//    if(!m_syncTimeFuture.isRunning() && m_currentTime > 0 && !m_stopping)
//        syncTime();
//#endif
}

void TimeLine::stop() {
    m_syncTimeRunning = false;
    m_syncTimeFuture.cancel();
    m_stopping = true;
    m_syncTimeFuture.waitForFinished();
    QTimer::singleShot(500, [this]() {
        m_stopping = false;
    });
    update();
}

void TimeLine::syncTime() {
    m_syncTimeFuture = QtConcurrent::run([this]()
    {
        qint64 timeTracker = 0;
        qint64 lastCurrentTime = 0;
        QElapsedTimer mSecTimer;
        qint64 timer1 = 0;
        qint64 timer2 = 0;
        mSecTimer.start();
        m_syncTimeRunning = true;
        while (m_syncTimeRunning)
        {
            if(!m_isPaused) {
                if (timer2 - timer1 >= 10)
                {
                    timer1 = timer2;
                    if(lastCurrentTime != m_currentTime)
                    {
                        //LogHandler::Debug("VR time reset: "+QString::number(currentTime));
                        lastCurrentTime = m_currentTime;
                        timeTracker = m_currentTime;
                    }
                    else
                    {
                        timeTracker++;
                        m_currentTime = timeTracker;
                    }
                    emit updatePaint();
                }
                timer2 = (round(mSecTimer.nsecsElapsed() / 1000000));
                //LogHandler::Debug("timer nsecsElapsed: "+QString::number(timer2));
            }
        }
        //LogHandler::Debug("exit syncTime");
    });
}

qint64 TimeLine::getCurrentTime() const {
    return m_currentTime;
}

QRectF TimeLine::firstHandleRect() const
{
    return handleRect(getTimeToWidth(m_startLoop), m_LoopHandleWidth);
}

QRectF TimeLine::currentHandleRect() const
{
    return handleRect(getTimeToWidth(m_currentTime) - m_HandleWidth / 2, m_HandleWidth);
}

QRectF TimeLine::secondHandleRect() const
{
    return handleRect(getTimeToWidth(m_endLoop) - m_LoopHandleWidth, m_LoopHandleWidth);
}

QRectF TimeLine::handleRect(qint64 aValue, int width) const {
    // left top width height
    return QRectF(aValue, 0, width, height());
}

int TimeLine::getTimeToWidth(qint64 time) const {
    return XMath::mapRange(time, (qint64)0, m_duration, (qint64)0, (qint64)width());
}

qint64 TimeLine::getPositionToTime(int position) const {
    return XMath::mapRange((qint64)position, (qint64)0, (qint64)width(), (qint64)0, m_duration);
}

bool TimeLine::getMousePressed() {
    return m_endLoopPressed || m_startLoopPressed || m_currentTimePressed;
}

void TimeLine::setLoop(bool enabled) {
    isLoop = enabled;
    if(isLoop && m_endLoop <= 0)
        m_endLoop = m_duration;
    update();
}

void TimeLine::togglePause(bool isPaused) {
    m_isPaused = isPaused;
}

void TimeLine::setLoopRange(qint64 loopStart, qint64 loopEnd) {
    if(loopEnd < 0) {
        loopEnd = 1000;
    }
    m_startLoop = loopStart;
    m_endLoop = loopEnd;
    update();
}

void TimeLine::clear() {
    m_startLoop = 0;
    m_endLoop = 0;
    isLoop = false;
    m_duration = 0;
    m_syncTimeRunning = false;
    m_syncTimeFuture.cancel();
    m_syncTimeFuture.waitForFinished();
    update();
}

void TimeLine::mousePressEvent(QMouseEvent* aEvent)
{
    if (!isEnabled())
    {
        return;
    }
    if(aEvent->buttons() & Qt::LeftButton)
    {
        emit mousePress();
        qint64 posCheck, posMax, posValue;
        posCheck = aEvent->pos().y();
        posMax = height();
        posValue = aEvent->pos().x();
        auto firstHandleRect = this->firstHandleRect();
        auto secondHandleRect = this->secondHandleRect();
        auto currentHandleRect = this->currentHandleRect();
//        currentHandleRect.setLeft(currentHandleRect.left() + currentHandleRect.left() * 0.75);
//        currentHandleRect.setRight(currentHandleRect.right() + currentHandleRect.right() * 0.75);

        m_endLoopPressed = secondHandleRect.contains(aEvent->pos());
        m_startLoopPressed = !m_endLoopPressed && firstHandleRect.contains(aEvent->pos());
        m_currentTimePressed = !m_startLoopPressed && !m_endLoopPressed && currentHandleRect.contains(aEvent->pos());

        if(posCheck >= 2 && posCheck <= posMax - 2)
        {
            qint64 destinationValue = getPositionToTime(posValue);
            if((m_currentTimePressed && m_duration > 0) || (!m_startLoopPressed && !m_endLoopPressed && m_duration > 0)) {
                if((isLoop && firstHandleRect.x() > posValue) || (isLoop && secondHandleRect.x() < posValue)) {
                    return;
                }
                setCurrentTime(destinationValue);
                emit currentTimeChanged(destinationValue);
            }
        }
    }
}

void TimeLine::mouseMoveEvent(QMouseEvent* aEvent)
{
    if (!isEnabled())
    {
        return;
    }
    if(aEvent->buttons() & Qt::LeftButton)
    {
        qint64 posValue, firstHandleRectPosValue, secondHandleRectPosValue;
        posValue = aEvent->pos().x();
        if(posValue < width() && posValue > 0) {
            firstHandleRectPosValue = firstHandleRect().x();
            secondHandleRectPosValue = secondHandleRect().x();

            if(m_currentTimePressed )
            {
                if(isLoop && posValue < firstHandleRectPosValue) {
                    setCurrentTime(getPositionToTime(firstHandleRectPosValue));
                } if(isLoop && posValue > secondHandleRectPosValue) {
                    setCurrentTime(getPositionToTime(secondHandleRectPosValue));
                } else {
                    setCurrentTime(getPositionToTime(posValue));
                }
                emit currentTimeChanged(m_currentTime);
            }
            else if(isLoop && m_startLoopPressed)
            {
                if(posValue + m_LoopHandleWidth <= secondHandleRectPosValue)
                {
                    setStartLoop(getPositionToTime(posValue));
                    emit startLoopMove(m_startLoop);
                }
                else
                {
                    setStartLoop(m_endLoop - m_minimumRange);
                    emit startLoopMove(m_startLoop);
                }
            }
            else if(isLoop && m_endLoopPressed )
            {
                if(firstHandleRectPosValue + m_LoopHandleWidth <= posValue)
                {
                    setEndLoop(getPositionToTime(posValue));
                    emit endLoopMove(m_endLoop);
                }
                else
                {
                    setEndLoop(m_startLoop + m_minimumRange);
                    emit endLoopMove(m_endLoop);
                }
            }
        }
    }


    //const int o = style()->pixelMetric(QStyle::PM_SliderLength ) - 1;
    //int sliderValue = QStyle::sliderValueFromPosition(0, m_duration, aEvent->pos().x()-o/2, width()-o, false);
    emit onHover(aEvent->x(), getPositionToTime(aEvent->pos().x()));
}

void TimeLine::enterEvent(QEvent * event)
{

}

void TimeLine::leaveEvent(QEvent * event)
{
    emit onLeave();
}
void TimeLine::mouseReleaseEvent(QMouseEvent* aEvent)
{
    Q_UNUSED(aEvent);

    m_startLoopPressed = false;
    m_endLoopPressed = false;
    m_currentTimePressed = false;
    emit mouseRelease(_name);
}

void TimeLine::changeEvent(QEvent* aEvent)
{
    if(aEvent->type() == QEvent::EnabledChange)
    {
        //updateColor();
    }
}

//QSize TimeLine::minimumSizeHint() const
//{
//    return QSize(m_HandleWidth * 2 * 2, m_HandleWidth);
//}

void TimeLine::setName(QString name)
{
    _name = name;
}

void TimeLine::setPaintBackground(bool isPainted) {
    m_isBackgroundPainted = isPainted;
}

qint64 TimeLine::getStartLoop() const
{
    return m_startLoop;
}

void TimeLine::setStartLoop(qint64 startLoop)
{
    if(startLoop > m_duration && m_duration > 0)
    {
        startLoop = m_duration - m_minimumRange;
    }
    else if(startLoop < 0)
    {
        startLoop = 0;
    }

    m_startLoop = startLoop;
    emit startLoopChanged(m_startLoop);

    update();
}


qint64 TimeLine::getEndLoop() const
{
    return m_endLoop;
}

void TimeLine::setEndLoop(qint64 endLoop)
{
    if(endLoop > m_duration && m_duration > 0)
    {
        endLoop = m_duration;
    }
    else if(endLoop < 0)
    {
        endLoop = 0 + m_minimumRange;
    }

    m_endLoop = endLoop;
    emit endLoopChanged(m_endLoop);

    update();
}

//int TimeLine::validLength() const
//{
//    int len = width();
//    return len * 2 - m_HandleWidth * 2;
//}

qint64 TimeLine::GetMinimumRange()
{
    return m_minimumRange;
}
