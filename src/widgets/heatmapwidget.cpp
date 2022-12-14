#include "heatmapwidget.h"
//#include <QSvgGenerator>
HeatmapWidget::HeatmapWidget(QWidget *parent)
    : QLabel{parent},
    painter(this)
{
    setMinimumSize(QSize(parent->minimumWidth(), 35));
    //setSizePolicy(QSizePolicy::QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
}
HeatmapWidget::~HeatmapWidget() {
}
void HeatmapWidget::paintEvent(QPaintEvent* event) {
//    QSvgGenerator svg;
//    painter.begin(&svg);
    painter.begin(this);
    painter.setBrush(QBrush(Qt::black));
    painter.fillRect(rect(), painter.brush());
    if(m_actions.count() > 0 && m_duration > 0) {
        m_heatMap.paint(&painter, width(), height() - 10, m_duration, m_actions, 5);
        int pos = XMath::mapRange(m_currentTime, (qint64)0, m_duration, (qint64)0, (qint64)width());
        painter.setPen(m_CurrentPositionPen);
        painter.drawLine(pos, 0, pos, height());
    } else {
        QLabel::clear();
        QLabel::paintEvent(event);
    }
    painter.end();
}
void HeatmapWidget::mouseReleaseEvent(QMouseEvent* event) {
    if(m_duration > 0) {
        int pos = XMath::mapRange((int)event->localPos().x(), 0, width(), 0, 100);
        emit mouseReleased(pos);
    }
}
void HeatmapWidget::resizeEvent(QResizeEvent *event) {
    //setPixmap(m_heatMapPixmap.scaled(event->size().width(), height(), Qt::IgnoreAspectRatio));
}

void HeatmapWidget::setDuration(qint64 duration) {
    m_duration = duration;
    repaint();
}

void HeatmapWidget::setCurrentTime(qint64 time) {
    m_currentTime = time;
    repaint();
}
void HeatmapWidget::setActions(QHash<qint64, int> actions) {
    QMap<qint64, int> sortedMap;
    foreach (auto key, actions.keys()) {
        sortedMap.insert(key, actions.value(key));
    }
    m_actions = sortedMap;
    repaint();
}
void HeatmapWidget::clearMap() {
    m_actions.clear();
    m_duration = 0;
    m_currentTime = 0;
    clear();
    repaint();
}
//void HeatmapWidget::setHeatMap(QPixmap map) {
////    m_heatMapPixmap = map;
////    setPixmap(map);
//}
