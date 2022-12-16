#include "heatmapwidget.h"
//#include <QSvgGenerator>
HeatmapWidget::HeatmapWidget(QWidget *parent)
    : QLabel{parent},
    painter(this) {
    setMinimumSize(QSize(parent->minimumWidth(), 35));
//    connect(&m_resizeDebounce, &QTimer::timeout, this, [this](){
//        paint();
//    });
    m_resizeDebounce.setSingleShot(true);
    paint();
    //setSizePolicy(QSizePolicy::QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
}

HeatmapWidget::~HeatmapWidget() {

}

//void HeatmapWidget::paintEvent(QPaintEvent* event) {
//    if(m_actions.count() > 0 && m_duration > 0 && m_heatMapPixmap.isNull()) {
////        painter.begin(this);
////        painter.setBrush(QBrush(Qt::black));
////        painter.fillRect(rect(), painter.brush());
//        QPixmap pixmap(width(), height());
//        m_heatMapPixmap.swap(pixmap);
//        //painter.setBrush(QBrush(Qt::black));
//        m_heatMapPixmap.fill(Qt::GlobalColor::black);
//        if(!painter.begin(&m_heatMapPixmap)) {
//            LogHandler::Error("Could not create painter for heat map");
//            return;
//        }
//        painter.setRenderHint( QPainter::Antialiasing, true);
//        m_heatMap.paint(&painter, width(), height() - 10, m_duration, m_actions, 5);
//        setPixmap(m_heatMapPixmap);
//        painter.end();
//    }
//    else if(m_heatMapPixmap.isNull()) {
//        painter.begin(this);
//        painter.setBrush(QBrush(Qt::black));
//        painter.fillRect(rect(), painter.brush());
//        painter.end();
//    }
//}
void HeatmapWidget::paint() {
    QPixmap pixmap(width(), height());
    if(m_actions.count() > 0 && m_duration > 0) {
//        painter.begin(this);
//        painter.setBrush(QBrush(Qt::black));
//        painter.fillRect(rect(), painter.brush());
        m_heatMapPixmap.swap(pixmap);
        //painter.setBrush(QBrush(Qt::black));
        m_heatMapPixmap.fill(Qt::GlobalColor::black);
        if(!painter.begin(&m_heatMapPixmap)) {
            LogHandler::Error("Could not create painter for heat map");
            return;
        }
        //painter.setRenderHint( QPainter::Antialiasing, true);
        m_heatMap.paint(&painter, width(), height() - 10, m_duration, m_actions, 5);
        setPixmap(m_heatMapPixmap);
        painter.end();
    } else {
        m_heatMapPixmap.swap(pixmap);
        m_heatMapPixmap.fill(Qt::GlobalColor::black);
        setPixmap(m_heatMapPixmap);
    }
}
void HeatmapWidget::resizeEvent(QResizeEvent *event) {
//    m_resizeDebounce.start(100);
    paint();
}

void HeatmapWidget::setDuration(qint64 duration) {
    m_duration = duration;
    paint();
}

void HeatmapWidget::setActions(QHash<qint64, int> actions) {
    QMap<qint64, int> sortedMap;
    foreach (auto key, actions.keys()) {
        sortedMap.insert(key, actions.value(key));
    }
    m_actions = sortedMap;
    paint();
}

void HeatmapWidget::clearMap() {
    m_actions.clear();
    m_duration = 0;
    m_currentTime = 0;
    paint();
}
