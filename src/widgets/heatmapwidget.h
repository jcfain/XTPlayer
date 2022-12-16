#ifndef HEATMAPWIDGET_H
#define HEATMAPWIDGET_H

#include <QLabel>
#include <QResizeEvent>
#include "lib/tool/heatmap.h"

class HeatmapWidget : public QLabel
{
    Q_OBJECT
signals:
    //void mouseReleased(int pos);
public:
    explicit HeatmapWidget(QWidget *parent = nullptr);
    ~HeatmapWidget();
    void setDuration(qint64 duration);
    //void setCurrentTime(qint64 duration);
    void setActions(QHash<qint64, int> actions);
    void clearMap();
    //void setHeatMap(QPixmap map);
protected:
//    void paintEvent(QPaintEvent* event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    void paint();
    QPainter painter;
    HeatMap m_heatMap;
    qint64 m_duration = 0;
    qint64 m_currentTime = 0;
    //QPixmap m_heatMapPixmap;
    QMap<qint64, int> m_actions;

    QTimer m_resizeDebounce;
   // QPen m_CurrentPositionPen = QPen(Qt::GlobalColor::cyan, 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);



};

#endif // HEATMAPWIDGET_H
