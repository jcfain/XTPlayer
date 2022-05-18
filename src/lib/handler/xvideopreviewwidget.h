#ifndef XVIDEOPREVIEWWIDGET_H
#define XVIDEOPREVIEWWIDGET_H

#include <QWidget>

class XVideoPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    XVideoPreviewWidget(QWidget* parent);
    void setFile(QString path);
    void setTimestamp(qint64 pos);
    void preview();
};

#endif // XVIDEOPREVIEWWIDGET_H
