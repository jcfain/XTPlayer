#ifndef XVIDEOPREVIEWWIDGET_H
#define XVIDEOPREVIEWWIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include "xvideosurface.h"

class XVideoPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    XVideoPreviewWidget(QWidget* parent);
    void setFile(QString path);
    void setTimestamp(qint64 pos);
    void preview();

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    QString _file;
    qint64 _time;
    XVideoSurface* _thumbNailVideoSurface;
    QMediaPlayer* _thumbPlayer;
    QGridLayout* _layout;
    QLabel* _label;
    bool _thumbRetrieving = false;

    void getThumb(QString videoPath, qint64 time);
    void on_thumbCapture(QPixmap thumb);
    void on_mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void on_mediaStateChange(QMediaPlayer::State state);

};

#endif // XVIDEOPREVIEWWIDGET_H
