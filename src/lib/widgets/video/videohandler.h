#ifndef VIDEOHANDLER_H
#define VIDEOHANDLER_H
#include <QWidget>
#include <QHBoxLayout>
#include <QMediaPlayer>
//#include <QAudioProbe>
#include <QVideoWidget>
#include <QMouseEvent>
#include <QMovie>
#include <QLabel>

#include "lib/handler/settingshandler.h"
#include "lib/handler/loghandler.h"
#include "lib/lookup/XMedia.h"

#include "xvideowidget.h"
#include "playercontrols.h"
#include "xlibrarylist.h"
#include "xwidget.h"
#include "dialoghandler.h"

class VideoHandler : public QWidget
{
    Q_OBJECT

private slots:
    void on_media_positionChanged(qint64 position);
    void on_media_statusChanged(QMediaPlayer::MediaStatus status);
    void on_media_stateChanged(QMediaPlayer::State state);
    void on_media_error(QMediaPlayer::Error error);
    void on_media_start();
    void on_media_stop();
//    void audioBufferProbed(const QAudioBuffer &buffer);

signals:
    void doubleClicked(QMouseEvent* e);
    void singleClicked(QMouseEvent* e);
    void keyPressed(QKeyEvent* k);
    void keyReleased(QKeyEvent* k);
    void mouseEnter(QEvent* e);
    void positionChanged(qint64 position);
    void mediaStatusChanged(XMediaStatus status);
    void mediaStateChanged(XMediaState state);
    void started();
    void stopped();
    void playing();
    void stopping();
    void togglePaused(bool paused);
    void durationChange(qint64 value);

public:;
    VideoHandler(PlayerControls* controls, XLibraryList* libraryList, QWidget* parent = 0);
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
    bool isFullScreen();
    void showFullscreen(QSize screenSize, bool libraryWindowed);
    void showNormal();
    void placeLibraryList(bool libraryWindowed = false);
    QGridLayout* libraryListLayout();

private:
    QGridLayout* _mediaGrid = 0;
    QFrame* m_libraryListFrame;
    QMediaPlayer* _player = 0;
    QTimer m_overlayTimer;
    //QAudioProbe* m_audioProbe = 0;
    bool _isFullScreen = false;
    bool m_libraryWindowed;
    QSize m_screenSize;
    XWidget* _fullscreenWidget;
//    VideoRenderer* _videoRenderer = 0;
//    VideoPreviewWidget* _videoPreviewWidget;
    QRect m_libraryRect;
    QRect m_controlsRect;
    XVideoWidget* _videoWidget;
    QString _currentFile;
    QMutex _mutex;
    qreal volumeBeforeMute;
    QLabel* _videoLoadingLabel = 0;
    QMovie* _videoLoadingMovie = 0;
    XMediaState _currentState = XMediaState::Stopped;

    void createLayout();

    void on_setLoading(bool loading);
    XMediaStatus convertMediaStatus(QMediaPlayer::MediaStatus status);
    XMediaState convertMediaState(QMediaPlayer::State status);

    PlayerControls* m_controls;
    XLibraryList* m_libraryList;

    void mouseMove( QMouseEvent* e );
    void hideControls();
    void showControls();
    void hideLibrary();
    void showLibrary();
    void hideControlsTimeout();
    void hideLibraryTimeout();
};
#endif // VIDEOHANDLER_H
