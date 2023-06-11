#include "playercontrols.h"

PlayerControls::PlayerControls(QWidget *parent, Qt::WindowFlags f) : QFrame(parent, f)
{
    setObjectName(QString::fromUtf8("playerControlsFrame"));
    setMinimumSize(QSize(240, 110));
    setMaximumSize(QSize(16777215, 16777215));
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    playerControlsGrid = new QGridLayout(this);
    playerControlsGrid->setObjectName(QString::fromUtf8("playerControlsGrid"));

    setLayout(playerControlsGrid);

    int row = 0;

    lblCurrentTime = new QLabel(this);
    lblCurrentTime->setObjectName(QString::fromUtf8("lblCurrentTime"));
    lblCurrentTime->setAlignment(Qt::AlignLeft|Qt::AlignBottom);

    playerControlsGrid->addWidget(lblCurrentTime, row, 0, 1, 1);

    skipToActionButton = new QPushButton(this);
    skipToActionButton->setObjectName(QString::fromUtf8("skipToActionButton"));
    skipToActionButton->setProperty("cssClass", "playerControlButton");
    skipToActionButton->setMinimumSize(QSize(20, 15));
    skipToActionButton->setToolTip("Skips to 1 second before the next funscript action.");
    QIcon iconActionBegin;
    iconActionBegin.addFile(QString::fromUtf8(":/images/icons/skipToAction.png"), QSize(), QIcon::Normal, QIcon::Off);
    skipToActionButton->setIcon(iconActionBegin);
    skipToActionButton->setIconSize(QSize(15, 15));
    skipToActionButton->setFlat(true);
    skipToActionButton->setEnabled(false);

    playerControlsGrid->addWidget(skipToActionButton, row, 1, 1, 3);

    alternateStriptCmb = new QComboBox(this);
    alternateStriptCmb->setEditable(false);
    playerControlsGrid->addWidget(alternateStriptCmb, row, 4, 1, 4);
    connect(alternateStriptCmb, &QComboBox::currentTextChanged, this, &PlayerControls::onAlternateFunscriptSelected);

    skipToMoneyShotButton = new QPushButton(this);
    skipToMoneyShotButton->setObjectName(QString::fromUtf8("skipToMoneyShotButton"));
    skipToMoneyShotButton->setProperty("cssClass", "playerControlButton");
    skipToMoneyShotButton->setMinimumSize(QSize(20, 15));
    skipToMoneyShotButton->setToolTip("Skips to the last 10% of the video by default. You can chenge this by right clicking the library item.\nYou can also assign a script to this action on the funscript tab in settings.");
    QIcon iconMoneyShot;
    iconMoneyShot.addFile(QString::fromUtf8(":/images/icons/skipToMoneyShot.svg"), QSize(), QIcon::Normal, QIcon::Off);
    skipToMoneyShotButton->setIcon(iconMoneyShot);
    skipToMoneyShotButton->setIconSize(QSize(15, 15));
    skipToMoneyShotButton->setFlat(true);
    skipToMoneyShotButton->setEnabled(false);

    playerControlsGrid->addWidget(skipToMoneyShotButton, row, 8, 1, 3);

    lblDuration = new QLabel(this);
    lblDuration->setObjectName(QString::fromUtf8("lblDuration"));
    lblDuration->setAlignment(Qt::AlignRight|Qt::AlignBottom);

    playerControlsGrid->addWidget(lblDuration, row, 11, 1, 1);

    row++;

//    SeekSlider = new RangeSlider(this);
//    SeekSlider->setObjectName(QString::fromUtf8("SeekSlider"));
//    SeekSlider->setEnabled(false);
//    SeekSlider->setMinimumSize(QSize(100, 15));
//    SeekSlider->setMaximum(100);
//    SeekSlider->setOrientation(Qt::Horizontal);
//    SeekSlider->setDisabled(true);
//    SeekSlider->SetRange(0, 100);
//    SeekSlider->setOption(RangeSlider::Option::RightHandle);
//    SeekSlider->setUpperValue(0);

//    playerControlsGrid->addWidget(SeekSlider, row, 0, 1, 12);

    row++;

    lblHeatmap = new HeatmapWidget(this);
    playerControlsGrid->addWidget(lblHeatmap, row, 0, 1, 12);

    m_timeLine = new TimeLine(this);
    m_timeLine->setObjectName(QString::fromUtf8("timeLine"));
    m_timeLine->setEnabled(false);
    //m_timeLine->setMinimumSize(QSize(100, 15));
    //m_timeLine->setMaximum(100);
    //m_timeLine->setOrientation(Qt::Horizontal);
    //m_timeLine->setDisabled(true);
    //m_timeLine->SetRange(0, 100);
    //m_timeLine->setOption(RangeSlider::Option::RightHandle);
    //m_timeLine->setUpperValue(0);

    playerControlsGrid->addWidget(m_timeLine, row, 0, 1, 12);

    on_heatmapToggled(XTPSettings::getHeatmapDisabled());

    row++;

    fullScreenBtn = new QPushButton(this);
    fullScreenBtn->setObjectName(QString::fromUtf8("fullScreenBtn"));
    fullScreenBtn->setProperty("cssClass", "playerControlButton");
    fullScreenBtn->setEnabled(true);
    fullScreenBtn->setMinimumSize(QSize(0, 20));
    QIcon icon4;
    icon4.addFile(QString::fromUtf8(":/images/icons/fullscreen.svg"), QSize(), QIcon::Normal, QIcon::Off);
    fullScreenBtn->setIcon(icon4);
    fullScreenBtn->setIconSize(QSize(20, 20));
    fullScreenBtn->setFlat(true);

    playerControlsGrid->addWidget(fullScreenBtn, row, 0, 1, 1);

    loopToggleButton = new QPushButton(this);
    loopToggleButton->setObjectName(QString::fromUtf8("loopToggleButton"));
    loopToggleButton->setProperty("cssClass", "playerControlButton");
    loopToggleButton->setMinimumSize(QSize(20, 20));
    QIcon icon3;
    icon3.addFile(QString::fromUtf8(":/images/icons/loop.svg"), QSize(), QIcon::Normal, QIcon::Off);
    loopToggleButton->setIcon(icon3);
    loopToggleButton->setIconSize(QSize(20, 20));
    loopToggleButton->setCheckable(true);
    loopToggleButton->setFlat(true);
    loopToggleButton->setEnabled(false);

    playerControlsGrid->addWidget(loopToggleButton, row, 1, 1, 1);

//    settingsButton = new QPushButton(this);
//    settingsButton->setObjectName(QString::fromUtf8("settingsButton"));
//    settingsButton->setProperty("cssClass", "playerControlButton");
//    settingsButton->setMinimumSize(QSize(0, 20));
//    QIcon icon5;
//    icon5.addFile(QString::fromUtf8(":/images/icons/settings.svg"), QSize(), QIcon::Normal, QIcon::Off);
//    settingsButton->setIcon(icon5);
//    settingsButton->setIconSize(QSize(20, 20));
//    settingsButton->setFlat(true);

//    playerControlsGrid->addWidget(settingsButton, row, 1, 1, 1);

    skipBackButton = new QPushButton(this);
    skipBackButton->setObjectName(QString::fromUtf8("skipBackButton"));
    skipBackButton->setProperty("cssClass", "playerControlButton");
    skipBackButton->setMinimumSize(QSize(20, 20));
    QIcon icon6;
    icon6.addFile(QString::fromUtf8(":/images/icons/skipBack.svg"), QSize(), QIcon::Normal, QIcon::Off);
    skipBackButton->setIcon(icon6);
    skipBackButton->setIconSize(QSize(20, 20));
    skipBackButton->setFlat(true);

    playerControlsGrid->addWidget(skipBackButton, row, 4, 1, 1);

    _stopBtn = new QPushButton(this);
    _stopBtn->setObjectName(QString::fromUtf8("stopBtn"));
    _stopBtn->setProperty("cssClass", "playerControlButton");
    _stopBtn->setMinimumSize(QSize(20, 20));
    QIcon iconStop;
    iconStop.addFile(QString::fromUtf8(":/images/icons/stop.svg"), QSize(), QIcon::Normal, QIcon::Off);
    _stopBtn->setIcon(iconStop);
    _stopBtn->setIconSize(QSize(20, 20));
    _stopBtn->setFlat(true);
    _stopBtn->setEnabled(false);

    playerControlsGrid->addWidget(_stopBtn, row, 5, 1, 1);

    PlayBtn = new QPushButton(this);
    PlayBtn->setObjectName(QString::fromUtf8("PlayBtn"));
    PlayBtn->setProperty("cssClass", "playerControlButton");
    PlayBtn->setMinimumSize(QSize(20, 20));
    QIcon icon7;
    icon7.addFile(QString::fromUtf8(":/images/icons/play.svg"), QSize(), QIcon::Normal, QIcon::Off);
    PlayBtn->setIcon(icon7);
    PlayBtn->setIconSize(QSize(20, 20));
    PlayBtn->setFlat(true);

    playerControlsGrid->addWidget(PlayBtn, row, 6, 1, 1);

    skipForwardButton = new QPushButton(this);
    skipForwardButton->setObjectName(QString::fromUtf8("skipForwardButton"));
    skipForwardButton->setProperty("cssClass", "playerControlButton");
    skipForwardButton->setMinimumSize(QSize(20, 20));
    QIcon icon1;
    icon1.addFile(QString::fromUtf8(":/images/icons/skipForward.svg"), QSize(), QIcon::Normal, QIcon::Off);
    skipForwardButton->setIcon(icon1);
    skipForwardButton->setIconSize(QSize(20, 20));
    skipForwardButton->setFlat(true);

    playerControlsGrid->addWidget(skipForwardButton, row, 7, 1, 1);

    VolumeSlider = new RangeSlider(this);
    VolumeSlider->setObjectName(QString::fromUtf8("VolumeSlider"));
    VolumeSlider->setMinimumSize(QSize(20, 20));
    VolumeSlider->setOrientation(Qt::Horizontal);
    VolumeSlider->setDisabled(false);
    VolumeSlider->SetRange(0, 100);
    VolumeSlider->setOption(RangeSlider::Option::RightHandle);

    playerControlsGrid->addWidget(VolumeSlider, row, 8, 1, 3);

    MuteBtn = new QPushButton(this);
    MuteBtn->setObjectName(QString::fromUtf8("muteBtn"));
    MuteBtn->setProperty("cssClass", "playerControlButton");
    MuteBtn->setMinimumSize(QSize(20, 20));
    QIcon icon2;
    icon2.addFile(QString::fromUtf8(":/images/icons/speakerMute.svg"), QSize(), QIcon::Normal, QIcon::Off);
    MuteBtn->setIcon(icon2);
    MuteBtn->setIconSize(QSize(20, 20));
    MuteBtn->setCheckable(true);
    MuteBtn->setFlat(true);

    playerControlsGrid->addWidget(MuteBtn, row, 11, 1, 1);

    connect(m_timeLine, &TimeLine::currentTimeChanged, this, &PlayerControls::on_timeline_currentTimeMove);
    connect(m_timeLine, &TimeLine::onHover, this, &PlayerControls::on_seekslider_hover);
    connect(m_timeLine, &TimeLine::onLeave, this, &PlayerControls::on_seekslider_leave);
    connect(VolumeSlider, &RangeSlider::upperValueMove, this, &PlayerControls::on_VolumeSlider_valueChanged);
    connect(PlayBtn, &QPushButton::clicked, this, &PlayerControls::on_PlayBtn_clicked);
    connect(_stopBtn, &QPushButton::clicked, this, &PlayerControls::on_StopBtn_clicked);
    connect(skipForwardButton, &QPushButton::clicked, this, [this]() {
        //lblHeatmap->clearMap();
        emit skipForward();
    });
    connect(skipToMoneyShotButton, &QPushButton::clicked, this, [this]() {emit skipToMoneyShot();});
    connect(skipToActionButton, &QPushButton::clicked, this, [this]() {emit skipToNextAction();});
    connect(skipBackButton, &QPushButton::clicked, this, [this]() {
        //lblHeatmap->clearMap();
        emit skipBack();
    });
    connect(fullScreenBtn, &QPushButton::clicked, this, [this]() {emit fullscreenToggled();});
    //connect(settingsButton, &QPushButton::clicked, this, [this]() {emit settingsClicked();});
    connect(loopToggleButton, &QPushButton::toggled, this, &PlayerControls::on_loopToggleButton_toggled);
    connect(MuteBtn, &QPushButton::toggled, this, &PlayerControls::on_MuteBtn_toggled);

    //connect(lblHeatmap, &HeatmapWidget::mouseReleased, this, &PlayerControls::seekSliderMoved);

    voulumeBeforeMute = SettingsHandler::getPlayerVolume();
    setVolume(voulumeBeforeMute);
    on_VolumeSlider_valueChanged(voulumeBeforeMute);
    updateTimeDurationLabels((qint64)0, (qint64)0);
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

void PlayerControls::togglePause(bool isPause)
{
    setPlayIcon(!isPause);
    m_timeLine->togglePause(isPause);
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
    setEndLoop(0);
    setTimeLineDisabled(!playing);
    setSkipToMoneyShotEnabled(playing);
    loopToggleButton->setEnabled(playing);
    _stopBtn->setEnabled(playing);
    setPlayIcon(playing);
    if(!playing)
    {
        updateTimeDurationLabels(0, 0);
        loopToggleButton->setChecked(false);
//        lblHeatmap->clearMap();
//        m_timeLine->clear();
        m_timeLine->stop();
        setAltScripts(QList<ScriptInfo>());
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
    return m_timeLine->pos();
}

int PlayerControls::getSeekSliderWidth()
{
    return m_timeLine->width();
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
        //qint64 currentVideoPositionPercentage = XMath::mapRange(currentPosition,  (qint64)0, currentDuration, (qint64)0, (qint64)100);
        m_timeLine->setStartLoop(currentPosition);
    }
    else if (loopToggleButton->isChecked() && _autoLoopOn)
    {
        _autoLoopOn = false;
        qint64 lowerValue = m_timeLine->getStartLoop();
        //qint64 currentVideoPositionPercentage = XMath::mapRange(currentPosition,  (qint64)0, currentDuration, (qint64)0, (qint64)100);
        m_timeLine->setEndLoop(currentPosition > lowerValue + m_timeLine->GetMinimumRange()
                                      ? currentPosition : currentPosition + lowerValue + m_timeLine->GetMinimumRange());
    }
    else
    {
        loopToggleButton->setChecked(false);
    }
}

void PlayerControls::setSeekSliderToolTip(qint64 timeCurrent)
{
    m_timeLine->setToolTip(mSecondFormat(timeCurrent));
}

void PlayerControls::setEndLoop(qint64 value)
{
    m_timeLine->setEndLoop(value);
}

qint64 PlayerControls::getEndLoop()
{
    return m_timeLine->getEndLoop();
}
void PlayerControls::setStartLoop(qint64 value)
{
    m_timeLine->setStartLoop(value);
}
qint64 PlayerControls::getStartLoop()
{
    return m_timeLine->getStartLoop();
}
//void PlayerControls::setLoopMinimumRange(qint64 value)
//{
//    return m_timeLine->SetMinimumRange(value);
//}
void PlayerControls::setTimeLineDisabled(bool disabled)
{
    m_timeLine->setDisabled(disabled);
}
bool PlayerControls::getTimeLineMousePressed() {
    return m_timeLine->getMousePressed();
}
void PlayerControls::updateTimeDurationLabels(qint64 time, qint64 duration)
{
    lblCurrentTime->setText(mSecondFormat(time));
    lblDuration->setText(mSecondFormat(duration));
}
void PlayerControls::setTime(qint64 time)
{
    lblCurrentTime->setText(mSecondFormat(time));
    m_timeLine->setCurrentTime(time);
}
void PlayerControls::setDuration(qint64 duration)
{
    m_duration = duration;
    lblHeatmap->setDuration(duration);
    m_timeLine->setDuration(duration);
}

QString PlayerControls::mSecondFormat(qint64 mSecs)
{
    if(mSecs <= 0)
        return "00:00:00";
    int seconds = mSecs / 1000;
    mSecs %= 1000;

    int minutes = seconds / 60;
    seconds %= 60;

    int hours = minutes / 60;
    minutes %= 60;
    QString hr = QString::number(hours);
    QString mn = QString::number(minutes);
    QString sc = QString::number(seconds);

    return (hr.length() == 1 ? "0" + hr : hr ) + ":" + (mn.length() == 1 ? "0" + mn : mn ) + ":" + (sc.length() == 1 ? "0" + sc : sc);
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
//    setPlayIcon(false);
//    lblHeatmap->clearMap();
//    m_timeLine->clear();
    emit stopClicked();
}

void PlayerControls::on_timeline_currentTimeMove(qint64 position)
{
    m_timeLine->setToolTip(QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")));
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

void PlayerControls::on_LoopRange_valueChanged(qint64 position)
{
    qint64 endLoop = m_timeLine->getEndLoop();
    qint64 startLoop = m_timeLine->getStartLoop();
    qint64 currentTime = m_timeLine->getCurrentTime();
    emit loopRangeChanged(currentTime, startLoop, endLoop);
}


void PlayerControls::on_loopToggleButton_toggled(bool checked)
{
    if (checked)
    {
//        if(m_duration > 0)
//            m_timeLine->setLoopRange(m_timeLine->getCurrentTime() <= 0 ? 0 : m_timeLine->getCurrentTime(), m_duration);
        connect(m_timeLine, &TimeLine::startLoopChanged, this, &PlayerControls::on_LoopRange_valueChanged);
        connect(m_timeLine, &TimeLine::endLoopChanged, this, &PlayerControls::on_LoopRange_valueChanged);
    }
    else
    {
        m_timeLine->setLoopRange(0, 0);
        disconnect(m_timeLine, QOverload<qint64>::of(&TimeLine::startLoopChanged), this, &PlayerControls::on_LoopRange_valueChanged);
        disconnect(m_timeLine, QOverload<qint64>::of(&TimeLine::endLoopChanged), this, &PlayerControls::on_LoopRange_valueChanged);
    }
    m_timeLine->setLoop(checked);
    emit loopButtonToggled(checked);
}

void PlayerControls::on_seekslider_hover(int position, qint64 sliderValue)
{
    emit seekSliderHover(position, sliderValue);
}
void PlayerControls::on_seekslider_leave()
{
    emit seekSliderLeave();
}

void PlayerControls::on_heatmapToggled(bool disabled) {
    if(disabled) {
        lblHeatmap->hide();
        lblHeatmap->clearMap();
    } else {
        lblHeatmap->show();
        lblHeatmap->setDuration(m_duration);
        lblHeatmap->setActions(m_actions);
    }
    m_timeLine->setPaintBackground(disabled);
}

void PlayerControls::setSkipToMoneyShotEnabled(bool enabled)
{
    skipToMoneyShotButton->setEnabled(enabled);
    skipToActionButton->setEnabled(enabled);
}

void PlayerControls::setActions(QHash<qint64, int> actions) {
    m_actions = actions;
    if(!XTPSettings::getHeatmapDisabled())
        lblHeatmap->setActions(actions);
}

void PlayerControls::onAlternateFunscriptSelected(QString script)
{
    auto scriptPath = alternateStriptCmb->currentData(Qt::UserRole).value<ScriptInfo>();
    emit alternateFunscriptSelected(scriptPath);
}

void PlayerControls::setAltScripts(QList<ScriptInfo> scriptInfos)
{

    disconnect(alternateStriptCmb, &QComboBox::currentTextChanged, this, &PlayerControls::onAlternateFunscriptSelected);
    alternateStriptCmb->clear();
    foreach(auto scriptInfo, scriptInfos) {
        if(!QFileInfo::exists(scriptInfo.path))
            continue;
        QString type = "";
        if(scriptInfo.containerType == ScriptContainerType::MFS)
            continue;
        else if(scriptInfo.containerType == ScriptContainerType::ZIP)
            type = " (ZIP)";
        alternateStriptCmb->addItem(scriptInfo.name + type, QVariant::fromValue(scriptInfo));
    }
    connect(alternateStriptCmb, &QComboBox::currentTextChanged, this, &PlayerControls::onAlternateFunscriptSelected);
}
