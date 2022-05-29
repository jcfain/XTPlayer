#ifndef VIDEOHANDLER_H
#define VIDEOHANDLER_H
#include <QWidget>
#include <QHBoxLayout>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QMouseEvent>
#include "lib/handler/settingshandler.h"
#include "lib/handler/loghandler.h"
enum XMediaStatus {
    LoadingMedia,
    LoadedMedia,
    NoMedia,
    BufferingMedia,
    BufferedMedia,
    UnknownMediaStatus,
    StalledMedia,
    InvalidMedia,
    EndOfMedia

};
enum XMediaState {
    Paused,
    Playing,
    Stopped,
};
class VideoHandler : public QVideoWidget
{
    Q_OBJECT

private slots:
    void on_media_positionChanged(qint64 position);
    void on_media_statusChanged(QMediaPlayer::MediaStatus status);
    void on_media_stateChanged(QMediaPlayer::State state);
    void on_media_error(QMediaPlayer::Error error);
    void on_media_start();
    void on_media_stop();

signals:
    void doubleClicked(QMouseEvent* e);
    void singleClicked(QMouseEvent* e);
    void keyPressed(QKeyEvent* k);
    void mouseEnter(QEvent* e);
    void positionChanged(int position);
    void mediaStatusChanged(XMediaStatus status);
    void started();
    void stopped();
    void playing();
    void stopping();
    void togglePaused(bool paused);

public:;
    VideoHandler(QWidget* parent = 0);
    virtual ~VideoHandler();
    bool isPlaying();
    void play();
    void stop();
    void togglePause();
    void pause();
    void setFile(QString file);
    QString file();
    void load();
    bool isMute();
    bool isPaused();
    void toggleMute();
    void setVolume(int value);
    XMediaState state();
    void seek(qint64 position);
    void setPosition(qint64 position);
    void setRepeat(int max = 0);
    void setSpeed(qreal speed);
    qint64 position();
    qint64 duration();
    QGridLayout* layout();
    void showPreview(int position, qint64 time);
    //void installFilter(AudioFilter* filter);
    void clearFilters();
    void setLoading(bool loading);
    QStringList getVideoExtensions();
    QStringList getAudioExtensions();
    void setFullscreen(bool on);
    void toggleFullscreen();

private:
    QWidget* _parent;
    QGridLayout* _mediaGrid = 0;
    QMediaPlayer* _player = 0;
//    VideoRenderer* _videoRenderer = 0;
//    VideoPreviewWidget* _videoPreviewWidget;
    //QVideoWidget* _videoWidget;
    QString _currentFile;
    QMutex _mutex;
    qreal volumeBeforeMute;
    QLabel* _videoLoadingLabel = 0;
    QMovie* _videoLoadingMovie = 0;
    XMediaState _currentState = XMediaState::Stopped;

    void createLayout();

    void mouseDoubleClickEvent(QMouseEvent * e) override;
    void mousePressEvent(QMouseEvent * e) override;
    void keyPressEvent(QKeyEvent * e) override;
    void enterEvent(QEvent * e) override;

    void on_setLoading(bool loading);
    XMediaStatus convertMediaStatus(QMediaPlayer::MediaStatus status);
    XMediaState convertMediaState(QMediaPlayer::State status);
};
#endif // VIDEOHANDLER_H
