#include "playercontrols.h"

PlayerControls::PlayerControls(QWidget *parent, Qt::WindowFlags f) : QFrame(parent, f)
{
    setObjectName(QString::fromUtf8("playerControlsFrame"));
    setMinimumSize(QSize(240, 75));
    setMaximumSize(QSize(16777215, 16777215));
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    playerControlsGrid = new QGridLayout(this);
    playerControlsGrid->setObjectName(QString::fromUtf8("playerControlsGrid"));

    setLayout(playerControlsGrid);

    fullScreenBtn = new QPushButton(this);
    fullScreenBtn->setObjectName(QString::fromUtf8("fullScreenBtn"));
    fullScreenBtn->setEnabled(true);
    fullScreenBtn->setMinimumSize(QSize(0, 20));
    QIcon icon4;
    icon4.addFile(QString::fromUtf8(":/images/icons/fullscreen.svg"), QSize(), QIcon::Normal, QIcon::Off);
    fullScreenBtn->setIcon(icon4);
    fullScreenBtn->setIconSize(QSize(20, 20));
    fullScreenBtn->setFlat(true);

    playerControlsGrid->addWidget(fullScreenBtn, 2, 0, 1, 1);

    settingsButton = new QPushButton(this);
    settingsButton->setObjectName(QString::fromUtf8("settingsButton"));
    settingsButton->setMinimumSize(QSize(0, 20));
    QIcon icon5;
    icon5.addFile(QString::fromUtf8(":/images/icons/settings.svg"), QSize(), QIcon::Normal, QIcon::Off);
    settingsButton->setIcon(icon5);
    settingsButton->setIconSize(QSize(20, 20));
    settingsButton->setFlat(true);

    playerControlsGrid->addWidget(settingsButton, 2, 1, 1, 1);

    skipToMoneyShotButton = new QPushButton(this);
    skipToMoneyShotButton->setObjectName(QString::fromUtf8("skipToMoneyShotButton"));
    skipToMoneyShotButton->setMinimumSize(QSize(0, 20));
    QIcon iconMoneyShot;
    iconMoneyShot.addFile(QString::fromUtf8(":/images/icons/skipToMoneyShot.svg"), QSize(), QIcon::Normal, QIcon::Off);
    skipToMoneyShotButton->setIcon(iconMoneyShot);
    skipToMoneyShotButton->setIconSize(QSize(20, 20));
    skipToMoneyShotButton->setFlat(true);
    skipToMoneyShotButton->setEnabled(false);

    playerControlsGrid->addWidget(skipToMoneyShotButton, 2, 2, 1, 1);

    skipBackButton = new QPushButton(this);
    skipBackButton->setObjectName(QString::fromUtf8("skipBackButton"));
    skipBackButton->setMinimumSize(QSize(0, 20));
    QIcon icon6;
    icon6.addFile(QString::fromUtf8(":/images/icons/skipBack.svg"), QSize(), QIcon::Normal, QIcon::Off);
    skipBackButton->setIcon(icon6);
    skipBackButton->setIconSize(QSize(20, 20));
    skipBackButton->setFlat(true);

    playerControlsGrid->addWidget(skipBackButton, 2, 4, 1, 1);

    PlayBtn = new QPushButton(this);
    PlayBtn->setObjectName(QString::fromUtf8("PlayBtn"));
    PlayBtn->setMinimumSize(QSize(0, 20));
    QIcon icon7;
    icon7.addFile(QString::fromUtf8(":/images/icons/play.svg"), QSize(), QIcon::Normal, QIcon::Off);
    PlayBtn->setIcon(icon7);
    PlayBtn->setIconSize(QSize(20, 20));
    PlayBtn->setFlat(true);

    playerControlsGrid->addWidget(PlayBtn, 2, 5, 1, 1);

    skipForwardButton = new QPushButton(this);
    skipForwardButton->setObjectName(QString::fromUtf8("skipForwardButton"));
    skipForwardButton->setMinimumSize(QSize(0, 20));
    QIcon icon1;
    icon1.addFile(QString::fromUtf8(":/images/icons/skipForward.svg"), QSize(), QIcon::Normal, QIcon::Off);
    skipForwardButton->setIcon(icon1);
    skipForwardButton->setIconSize(QSize(20, 20));
    skipForwardButton->setFlat(true);

    playerControlsGrid->addWidget(skipForwardButton, 2, 6, 1, 1);


    loopToggleButton = new QPushButton(this);
    loopToggleButton->setObjectName(QString::fromUtf8("loopToggleButton"));
    loopToggleButton->setMinimumSize(QSize(0, 20));
    QIcon icon3;
    icon3.addFile(QString::fromUtf8(":/images/icons/loop.svg"), QSize(), QIcon::Normal, QIcon::Off);
    loopToggleButton->setIcon(icon3);
    loopToggleButton->setIconSize(QSize(20, 20));
    loopToggleButton->setCheckable(true);
    loopToggleButton->setFlat(true);
    loopToggleButton->setEnabled(false);

    playerControlsGrid->addWidget(loopToggleButton, 2, 7, 1, 1);

    VolumeSlider = new RangeSlider(this);
    VolumeSlider->setObjectName(QString::fromUtf8("VolumeSlider"));
    VolumeSlider->setMinimumSize(QSize(0, 20));
    VolumeSlider->setOrientation(Qt::Horizontal);
    VolumeSlider->setDisabled(false);
    VolumeSlider->SetRange(0, 1000);
    VolumeSlider->setOption(RangeSlider::Option::RightHandle);

    playerControlsGrid->addWidget(VolumeSlider, 2, 8, 1, 2);

    MuteBtn = new QPushButton(this);
    MuteBtn->setObjectName(QString::fromUtf8("MuteBtn"));
    MuteBtn->setMinimumSize(QSize(0, 20));
    QIcon icon2;
    icon2.addFile(QString::fromUtf8(":/images/icons/speakerMute.svg"), QSize(), QIcon::Normal, QIcon::Off);
    MuteBtn->setIcon(icon2);
    MuteBtn->setIconSize(QSize(20, 20));
    MuteBtn->setCheckable(true);
    MuteBtn->setFlat(true);

    playerControlsGrid->addWidget(MuteBtn, 2, 10, 1, 1);

    lblCurrentTime = new QLabel(this);
    lblCurrentTime->setObjectName(QString::fromUtf8("lblCurrentTime"));
    lblCurrentTime->setAlignment(Qt::AlignLeft|Qt::AlignBottom);

    playerControlsGrid->addWidget(lblCurrentTime, 0, 0, 1, 1);

    SeekSlider = new RangeSlider(this);
    SeekSlider->setObjectName(QString::fromUtf8("SeekSlider"));
    SeekSlider->setEnabled(false);
    SeekSlider->setMinimumSize(QSize(100, 15));
    SeekSlider->setMaximum(100);
    SeekSlider->setOrientation(Qt::Horizontal);
    SeekSlider->setDisabled(true);
    SeekSlider->SetRange(0, 100);
    SeekSlider->setOption(RangeSlider::Option::RightHandle);
    SeekSlider->setUpperValue(0);

    playerControlsGrid->addWidget(SeekSlider, 1, 0, 1, 11);

    lblDuration = new QLabel(this);
    lblDuration->setObjectName(QString::fromUtf8("lblDuration"));
    lblDuration->setAlignment(Qt::AlignRight|Qt::AlignBottom);

    playerControlsGrid->addWidget(lblDuration, 0, 10, 1, 1);

    connect(SeekSlider, &RangeSlider::upperValueMove, this, &PlayerControls::on_seekSlider_sliderMoved);
    connect(SeekSlider, &RangeSlider::onHover, this, &PlayerControls::on_seekslider_hover);
    connect(SeekSlider, &RangeSlider::onLeave, this, &PlayerControls::on_seekslider_leave);
    connect(VolumeSlider, &RangeSlider::upperValueMove, this, &PlayerControls::on_VolumeSlider_valueChanged);
    connect(PlayBtn, &QPushButton::clicked, this, &PlayerControls::on_PlayBtn_clicked);
    connect(skipForwardButton, &QPushButton::clicked, this, [this]() {emit skipForward();});
    connect(skipToMoneyShotButton, &QPushButton::clicked, this, [this]() {emit skipToMoneyShot();});
    connect(skipBackButton, &QPushButton::clicked, this, [this]() {emit skipBack();});
    connect(fullScreenBtn, &QPushButton::clicked, this, [this]() {emit fullscreenToggled();});
    connect(settingsButton, &QPushButton::clicked, this, [this]() {emit settingsClicked();});
    connect(loopToggleButton, &QPushButton::toggled, this, &PlayerControls::on_loopToggleButton_toggled);
    connect(MuteBtn, &QPushButton::toggled, this, &PlayerControls::on_MuteBtn_toggled);

    voulumeBeforeMute = SettingsHandler::getPlayerVolume();
    setVolume(voulumeBeforeMute);
    on_VolumeSlider_valueChanged(voulumeBeforeMute);
    setTimeDuration("00:00:00", "00:00:00");
}

PlayerControls::~PlayerControls()
{

}

bool PlayerControls::getAutoLoop()
{
    return loopToggleButton->isChecked();
}

void PlayerControls::setVolume(int value)
{
    setVolumeIcon(value);
    VolumeSlider->setToolTip(QString::number(value));
    VolumeSlider->setUpperValue(value);
}

void PlayerControls::setPlayIcon(bool playing)
{
    QIcon icon(playing ? "://images/icons/pause.svg" : "://images/icons/play.svg" );
    PlayBtn->setIcon(icon);
}
void PlayerControls::setVolumeIcon(int volume)
{
    if (volume > VolumeSlider->GetMaximum() / 2)
        MuteBtn->setIcon(QIcon(":/images/icons/speakerLoud.svg"));
    else if (volume > 0)
    {
        MuteBtn->setIcon(QIcon(":/images/icons/speakerMid.svg"));
    }
    else
    {
        MuteBtn->setIcon(QIcon(":/images/icons/speakerMute.svg"));
    }
}
void PlayerControls::resetMediaControlStatus(bool playing)
{
    LogHandler::Debug("Enter toggleMediaControlStatus: "+QString::number(playing));
    setSeekSliderUpperValue(0);
    setSeekSliderDisabled(!playing);
    setSkipToMoneyShotEnabled(playing);
    loopToggleButton->setEnabled(playing);
    setPlayIcon(playing);
    if(!playing)
    {
        setTimeDuration("00:00:00", "00:00:00");
    }
}

void PlayerControls::IncreaseVolume()
{
    int currentVolume = VolumeSlider->GetUpperValue();
    int maxVolume = VolumeSlider->GetMaximum();
    int newVolume = currentVolume - maxVolume <= 5 ? currentVolume + 5 : maxVolume;
    setVolume(newVolume);
}

void PlayerControls::DecreaseVolume()
{
    int currentVolume = VolumeSlider->GetUpperValue();
    int minVolume = VolumeSlider->GetMinimum();
    int newVolume = currentVolume - minVolume  >= 5 ? currentVolume - 5 : minVolume;
    setVolume(newVolume);
}

QPoint PlayerControls::getTimeSliderPosition()
{
    return SeekSlider->pos();
}
int PlayerControls::getSeekSliderWidth()
{
    return SeekSlider->width();
}

void PlayerControls::SetLoop(bool loop)
{
    loopToggleButton->setChecked(loop);
}

void PlayerControls::toggleLoop(qint64 currentDuration, qint64 currentPosition)
{
    if(!loopToggleButton->isChecked() && !_autoLoopOn)
    {
        _autoLoopOn = true;
        loopToggleButton->setChecked(true);
        qint64 currentVideoPositionPercentage = XMath::mapRange(currentPosition,  (qint64)0, currentDuration, (qint64)0, (qint64)100);
        SeekSlider->setLowerValue(currentVideoPositionPercentage);
    }
    else if (loopToggleButton->isChecked() && _autoLoopOn)
    {
        _autoLoopOn = false;
        int lowerValue = SeekSlider->GetLowerValue();
        qint64 currentVideoPositionPercentage = XMath::mapRange(currentPosition,  (qint64)0, currentDuration, (qint64)0, (qint64)100);
        SeekSlider->setUpperValue(currentVideoPositionPercentage > lowerValue + SeekSlider->GetMinimumRange()
                                      ? currentVideoPositionPercentage : currentVideoPositionPercentage + lowerValue + SeekSlider->GetMinimumRange());
    }
    else
    {
        loopToggleButton->setChecked(false);
    }
}

void PlayerControls::setSeekSliderToolTip(QString timeCurrent)
{
    SeekSlider->setToolTip(timeCurrent);
}

void PlayerControls::setSeekSliderUpperValue(int value)
{
    SeekSlider->setUpperValue(value);
}

int PlayerControls::getSeekSliderUpperValue()
{
    return SeekSlider->GetUpperValue();
}
void PlayerControls::setSeekSliderLowerValue(int value)
{
    SeekSlider->setLowerValue(value);
}
int PlayerControls::getSeekSliderLowerValue()
{
    return SeekSlider->GetLowerValue();
}
void PlayerControls::setSeekSliderMinimumRange(int value)
{
    return SeekSlider->SetMinimumRange(value);
}
void PlayerControls::setSeekSliderDisabled(bool disabled)
{
    SeekSlider->setDisabled(disabled);
}
void PlayerControls::setTimeDuration(QString time, QString duration)
{
    lblCurrentTime->setText(time);
    lblDuration->setText(duration);
}
void PlayerControls::setTime(QString time)
{
    lblCurrentTime->setText(time);
}
void PlayerControls::setDuration(QString duration)
{
    lblDuration->setText(duration);
}


//Slots
void PlayerControls::on_VolumeSlider_valueChanged(int value)
{
    setVolumeIcon(value);
    VolumeSlider->setToolTip(QString::number(value));

    disconnect(MuteBtn, &QPushButton::toggled, this, &PlayerControls::on_MuteBtn_toggled);
    if(value > 0)
        MuteBtn->setChecked(false);
    else
        MuteBtn->setChecked(true);
    connect(MuteBtn, &QPushButton::toggled, this, &PlayerControls::on_MuteBtn_toggled);
    emit volumeChanged(value);
}

void PlayerControls::on_PlayBtn_clicked()
{
    emit playClicked();
}

void PlayerControls::on_StopBtn_clicked()
{
    setPlayIcon(false);
    emit stopClicked();
}

void PlayerControls::on_seekSlider_sliderMoved(int position)
{
    if (!getAutoLoop())
    {
        SeekSlider->setToolTip(QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")));
    }
    emit seekSliderMoved(position);
}

void PlayerControls::on_MuteBtn_toggled(bool checked)
{
    if (checked)
    {
        voulumeBeforeMute = VolumeSlider->GetUpperValue();
        setVolumeIcon(0);
        VolumeSlider->setUpperValue(0);
    }
    else
    {
        if(voulumeBeforeMute < 1)
            voulumeBeforeMute = 1;
        setVolumeIcon(voulumeBeforeMute);
        VolumeSlider->setUpperValue(voulumeBeforeMute);
    }
    emit muteChanged(checked);
}

void PlayerControls::on_LoopRange_valueChanged(int position)
{
    int endLoop = SeekSlider->GetUpperValue();
    int startLoop = SeekSlider->GetLowerValue();
    emit loopRangeChanged(position, startLoop, endLoop);
}


void PlayerControls::on_loopToggleButton_toggled(bool checked)
{
    if (checked)
    {
        SeekSlider->setOption(RangeSlider::Option::DoubleHandles);
        SeekSlider->SetRange(0, 100);
        SeekSlider->updateColor();
        connect(SeekSlider, QOverload<int>::of(&RangeSlider::lowerValueChanged), this, &PlayerControls::on_LoopRange_valueChanged);
        connect(SeekSlider, QOverload<int>::of(&RangeSlider::upperValueChanged), this, &PlayerControls::on_LoopRange_valueChanged);
    }
    else
    {
        SeekSlider->setOption(RangeSlider::Option::RightHandle);
        SeekSlider->SetRange(0, 100);
        SeekSlider->updateColor();
        disconnect(SeekSlider, QOverload<int>::of(&RangeSlider::lowerValueChanged), this, &PlayerControls::on_LoopRange_valueChanged);
        disconnect(SeekSlider, QOverload<int>::of(&RangeSlider::upperValueChanged), this, &PlayerControls::on_LoopRange_valueChanged);
    }
    emit loopButtonToggled(checked);
}

void PlayerControls::on_seekslider_hover(int position, int sliderValue)
{
    emit seekSliderHover(position, sliderValue);
}
void PlayerControls::on_seekslider_leave()
{
    emit seekSliderLeave();
}

void PlayerControls::setSkipToMoneyShotEnabled(bool enabled)
{
    skipToMoneyShotButton->setEnabled(enabled);
}
