#include "xvideopreviewwidget.h"

XVideoPreviewWidget::XVideoPreviewWidget(QWidget* parent) : QFrame(parent)
{

    //resize(176, 100);
    _layout = new QGridLayout(this);
    _label = new QLabel(this);
    _label->setAlignment(Qt::AlignHCenter);
    //_label->resize(170,94);
    //_label->setScaledContents(true);
    _layout->addWidget(_label, 0,0);

//    setWindowFlags(
//        Qt::Tool |
//        Qt::FramelessWindowHint |
//        Qt::WindowSystemMenuHint |
//        Qt::WindowStaysOnTopHint
//    );
    //setAttribute(Qt::WA_TranslucentBackground);
    setProperty("cssClass", "timeLinePreview");
    _videoLoadingMovie = new QMovie(this);
    _videoLoadingMovie->setFileName("://images/Eclipse-1s-loading-200px.gif");
    _videoLoadingLabel = new QLabel(this);
    _videoLoadingMovie->setScaledSize({50,50});
    _videoLoadingLabel->setMovie(_videoLoadingMovie);
    _videoLoadingLabel->setAttribute(Qt::WA_TransparentForMouseEvents );
    // _videoLoadingLabel->setStyleSheet("* {background: ffffff}");
    // _videoLoadingLabel->setProperty("cssClass", "mediaLoadingSpinner");
    _videoLoadingLabel->setAlignment(Qt::AlignCenter);
    //_videoLoadingLabel->setScaledContents(true);
    _layout->addWidget(_videoLoadingLabel, 0,0);
    _timeLabel = new QLabel(this);
    _timeLabel->setAlignment(Qt::AlignCenter);
    _layout->addWidget(_timeLabel, 2, 0);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
    setAttribute(Qt::WA_TransparentForMouseEvents );

    connect(&_videoPreview, &ThumbExtractor::frameExtracted, this, &XVideoPreviewWidget::on_thumbExtract);
}

void XVideoPreviewWidget::setFile(QString path) {
    _file = path;
}

void XVideoPreviewWidget::preview(QPoint gpos, qint64 time) {
    on_setLoading(true);
    _timeLabel->setText(QTime(0, 0, 0).addMSecs(time).toString(QString::fromLatin1("HH:mm:ss")));
    _videoPreview.extractDebounce(_file, time);
    _currentPosition = gpos;
    resize(100, 100);
    QPoint finalPos = gpos - QPoint(width()/2, height());
    move(finalPos);
    // LogHandler::Debug("[XVideoPreviewWidget] loadingPos x: " + QString::number(finalPos.x()) + ", y: " + QString::number(finalPos.y()));
    show();
}

void XVideoPreviewWidget::stop()
{
     _videoPreview.reset();
}

void XVideoPreviewWidget::on_thumbExtract(QImage frame) {
    _label->setToolTip("");
    auto scaled = QPixmap::fromImage(frame.scaled(_thumbSize,_thumbSize,Qt::KeepAspectRatio));
    frame = QImage();
    resize(scaled.width(), scaled.height());
    QPoint finalPos = _currentPosition - QPoint(width()/2, height() + 50);
    move(finalPos);
    // LogHandler::Debug("[XVideoPreviewWidget] previofinalPos x: " + QString::number(finalPos.x()) + ", y: " + QString::number(finalPos.y()));
    _label->setPixmap(scaled);
    _label->update();
    on_setLoading(false);
}

void XVideoPreviewWidget::on_thumbExtractionError(QString error) {
    _label->setToolTip(error);
    emit thumbExtractionError(error);
}

void XVideoPreviewWidget::closeEvent(QCloseEvent *event) {
  QWidget::closeEvent(event);
}

void XVideoPreviewWidget::on_setLoading(bool loading)
{
    if(loading && _videoLoadingMovie->state() != QMovie::MovieState::Running)
    {
        _label->hide();
        _videoLoadingLabel->show();
        _videoLoadingMovie->start();
    }
    else if(!loading && _videoLoadingMovie->state() == QMovie::MovieState::Running)
    {
        _videoLoadingLabel->hide();
        _videoLoadingMovie->stop();
        _label->show();
    }
}
