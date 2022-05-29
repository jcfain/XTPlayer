#ifndef XVIDEOPREVIEWWIDGET_H
#define XVIDEOPREVIEWWIDGET_H

#include <QWidget>
#include "xvideopreview.h"
#include "settingshandler.h"

class XVideoPreviewWidget : public QFrame
{
    Q_OBJECT
signals:
    void thumbExtractionError(QString error);

public:
    XVideoPreviewWidget(QWidget* parent);
    void setFile(QString path);
    void setTimestamp(qint64 time);
    void preview(QPoint gpos);

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    QGridLayout* _layout;
    QLabel* _label;
    QString _file;
    qint64 _time;
    XVideoPreview _videoPreview;
    QLabel* _videoLoadingLabel = 0;
    QMovie* _videoLoadingMovie = 0;
    QPoint _currentPosition;
    int _thumbSize = 200;

    void on_thumbExtract(QPixmap frame);
    void on_thumbExtractionError(QString error);
    void on_setLoading(bool loading);
};

#endif // XVIDEOPREVIEWWIDGET_H
