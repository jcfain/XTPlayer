#ifndef XVIDEOPREVIEW_H
#define XVIDEOPREVIEW_H

#include <QMediaPlayer>
#include "xvideosurface.h"

class XVideoPreview : public QObject
{
    Q_OBJECT
signals:
    void mediaLoaded();
    void durationChanged(qint64 duration);
    void frameExtracted(QPixmap frame);
    void frameExtractionError(QString error);

public:
    XVideoPreview(QObject* parent = nullptr);
    void extract(QString videoPath, qint64 time);
    void load(QString file);
    void extractLoaded(qint64 time);

private:
    XVideoSurface* _thumbNailVideoSurface;
    QMediaPlayer* _thumbPlayer;
    bool _thumbRetrieving = false;
    QString _file;
    qint64 _time;

    bool _loadingInfo = false;

    void setUpPlayer();
    void tearDownPlayer();
    void on_thumbCapture(QPixmap thumb);
    void on_mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void on_mediaStateChange(QMediaPlayer::State state);
    void on_thumbError(QString error);
    void on_durationChanged(qint64 duration);
};

#endif // XVIDEOPREVIEW_H
