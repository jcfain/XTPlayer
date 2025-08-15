#ifndef PLAYERCONTROLS_H
#define PLAYERCONTROLS_H

#include <QFrame>
#include <QGridLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QDialog>
#include <QDoubleSpinBox>

#include "lib/tool/xmath.h"
#include "lib/struct/ScriptInfo.h"

#include "rangeslider.h"
#include "xtpsettings.h"
#include "heatmapwidget.h"
#include "timeline.h"

class PlayerControls : public QFrame
{
    Q_OBJECT
signals:
    void loopButtonToggled(bool loop);
    void loopRangeChanged(int position, int startLoop, int endLoop);
    void playClicked();
    void stopClicked();
    void volumeChanged(int value);
    void subtitleChanged(int value);
    void skipForward();
    void skipToMoneyShot();
    void skipToNextAction();
    void skipBack();
    void fullscreenToggled();
    void settingsClicked();
    void muteChanged(bool muted);
    void seekSliderMoved(qint64 value);
    void seekSliderHover(QPoint position, qint64 sliderValue);
    void seekSliderLeave();
    void alternateFunscriptSelected(ScriptInfo script);
    void playbackSpeedValueChanged(qreal value);
    void editMetadataClicked(LibraryListItem27* item);
public slots:
    void on_heatmapToggled(bool disabled);
public:
    explicit PlayerControls(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~PlayerControls();

    void IncreaseVolume();
    void DecreaseVolume();
    void setPlayIcon(bool playing);
    void togglePause(bool isPause);
    void setVolume(int volume);
    void SetLoop(bool loop);
    void toggleLoop(qint64 currentDuration, qint64 currentPosition);
    bool getAutoLoop();
    QPoint getTimeSliderPosition();
    QPoint getTimeSliderGlobalPosition();
    int getSeekSliderWidth();
    void setSeekSliderToolTip(qint64 value);
    void setEndLoop(qint64 value);
    qint64 getEndLoop();
    void setStartLoop(qint64 value);
    qint64 getStartLoop();
    //void setLoopMinimumRange(qint64 value);
    void setTimeLineDisabled(bool value);
    bool getTimeLineMousePressed();
    void updateTimeDurationLabels(qint64 time, qint64 duration);
    void setTime(qint64 time);
    void setSubtitles(const QList<QString>& value);
    void setDuration(qint64 duration);
    void resetMediaControlStatus(bool playing);
    void setSkipToMoneyShotEnabled(bool enabled);
    void setActions(QHash<qint64, int> actions);
    void clearActions();
    void setAltScripts(QList<ScriptInfo> scriptInfos);
    bool alternateFunscriptNext();
    bool alternateFunscriptPrev();
    void setPlayBackSpeed(qreal value);

private:
    QGridLayout *playerControlsGrid;
    QPushButton *skipForwardButton;
    QPushButton *skipToMoneyShotButton;
    QComboBox* alternateStriptCmb;
    QComboBox* subtitleCmb;
    QPushButton *skipToActionButton;
    QPushButton *MuteBtn;
    QPushButton *loopToggleButton;
    QPushButton *fullScreenBtn;
    QPushButton *altScriptBtn;
    QPushButton *mediaSettingsBtn;
    QDoubleSpinBox* playbackRateInput;
    //QPushButton *settingsButton;
    RangeSlider *VolumeSlider;
    QDialog* altScriptDialog;
//    RangeSlider *SeekSlider;
    QLabel *lblCurrentTime;
    QLabel *lblDuration;
    HeatmapWidget *lblHeatmap;
    TimeLine* m_timeLine;
    QPushButton* skipBackButton;
    QPushButton* PlayBtn;
    QPushButton* _stopBtn;
    qint64 m_duration;
    QHash<qint64, int> m_actions;

    int voulumeBeforeMute;
    bool _autoLoopOn = false;

    void setVolumeIcon(int volume);
    QString mSecondFormat(qint64 seconds);

    //Slots
    void on_VolumeSlider_valueChanged(int value);
    void on_PlayBtn_clicked();
    void on_StopBtn_clicked();
    void on_MuteBtn_toggled(bool checked);
    void on_timeline_currentTimeMove(qint64 position);
    void on_LoopRange_valueChanged(qint64 position);
    void on_loopToggleButton_toggled(bool checked);
    void on_seekslider_hover(QPoint position, qint64 sliderValue);
    void on_seekslider_leave();
    void onAlternateFunscriptSelected(QString script);
    void onSubtitleSelected(QString script);

};

#endif // PLAYERCONTROLS_H
