#ifndef XVIDEOPREVIEWWIDGET_H
#define XVIDEOPREVIEWWIDGET_H

#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMovie>
#include "lib/handler/xvideopreview.h"
#include "lib/handler/settingshandler.h"

class XVideoPreviewWidget : public QFrame
{
    Q_OBJECT
signals:
    void thumbExtractionError(QString error);

public:
    XVideoPreviewWidget(QWidget* parent = nullptr);
    void setFile(QString path);
    void preview(QPoint gpos, qint64 time);
    void stop();

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    QGridLayout* _layout;
    QLabel* _label;
    QString _file;
    QLabel* _timeLabel;
    XVideoPreview _videoPreview;
    QLabel* _videoLoadingLabel = 0;
    QMovie* _videoLoadingMovie = 0;
    QPoint _currentPosition;
    int _thumbSize = 200;

    void on_thumbExtract(QImage frame);
    void on_thumbExtractionError(QString error);
    void on_setLoading(bool loading);
};

#endif // XVIDEOPREVIEWWIDGET_H
