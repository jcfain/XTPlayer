#ifndef PLAYERCONTROLS_H
#define PLAYERCONTROLS_H

#include <QFrame>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include "CustomControls/rangeslider.h"
#include "lib/handler/settingshandler.h"
#include "lib/tool/xmath.h"

class PlayerControls : public QFrame
{
    Q_OBJECT
signals:
    void loopButtonToggled(bool loop);
    void loopRangeChanged(int position, int startLoop, int endLoop);
    void playClicked();
    void stopClicked();
    void volumeChanged(int value);
    void skipForward();
    void skipBack();
    void fullscreenToggled();
    void settingsClicked();
    void muteChanged(bool muted);
    void seekSliderMoved(int value);
    void seekSliderHover(int position, int sliderValue);
    void seekSliderLeave();

public:
    explicit PlayerControls(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~PlayerControls();

    void IncreaseVolume();
    void DecreaseVolume();
    void setPlayIcon(bool playing);
    void setVolume(int volume);
    void SetLoop(bool loop);
    void toggleLoop(qint64 currentDuration, qint64 currentPosition);
    bool getAutoLoop();
    QPoint getTimeSliderPosition();
    int getSeekSliderWidth();
    void setSeekSliderToolTip(QString value);
    void setSeekSliderUpperValue(int value);
    int getSeekSliderUpperValue();
    void setSeekSliderLowerValue(int value);
    int getSeekSliderLowerValue();
    void setSeekSliderMinimumRange(int value);
    void setSeekSliderDisabled(bool value);
    void setTimeDuration(QString value);
    void resetMediaControlStatus(bool playing);


private:
    QGridLayout *playerControlsGrid;
    QPushButton *skipForwardButton;
    QPushButton *MuteBtn;
    QPushButton *loopToggleButton;
    QPushButton *fullScreenBtn;
    QPushButton *settingsButton;
    RangeSlider *VolumeSlider;
    RangeSlider *SeekSlider;
    QLabel *lblCurrentDuration;
    QPushButton *skipBackButton;
    QPushButton *PlayBtn;

    int voulumeBeforeMute;
    bool _autoLoopOn = false;

    void setVolumeIcon(int volume);

    //Slots
    void on_VolumeSlider_valueChanged(int value);
    void on_PlayBtn_clicked();
    void on_StopBtn_clicked();
    void on_MuteBtn_toggled(bool checked);
    void on_seekSlider_sliderMoved(int position);
    void on_LoopRange_valueChanged(int position);
    void on_loopToggleButton_toggled(bool checked);
    void on_seekslider_hover(int position, int sliderValue);
    void on_seekslider_leave();

};

#endif // PLAYERCONTROLS_H
