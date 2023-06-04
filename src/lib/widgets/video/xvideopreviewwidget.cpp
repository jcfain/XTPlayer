#include "xvideopreviewwidget.h"

XVideoPreviewWidget::XVideoPreviewWidget(QWidget* parent) : QFrame(parent)
{

    //resize(176, 100);
    _layout = new QGridLayout(this);
    _label = new QLabel(this);
    _label->setAlignment(Qt::AlignHCenter);
    //_label->resize(170,94);
    //_label->setScaledContents(true);
    setLayout(_layout);
    _layout->addWidget(_label);

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
    //_videoLoadingLabel->setStyleSheet("* {background: ffffff}");
    _videoLoadingLabel->setProperty("cssClass", "mediaLoadingSpinner");
    _videoLoadingLabel->setAlignment(Qt::AlignCenter);
    //_videoLoadingLabel->setScaledContents(true);
    _layout->addWidget(_videoLoadingLabel);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    connect(&_videoPreview, &XVideoPreview::frameExtracted, this, &XVideoPreviewWidget::on_thumbExtract);
}

void XVideoPreviewWidget::setFile(QString path) {
    _file = path;
}

void XVideoPreviewWidget::setTimestamp(qint64 pos) {
    _time = pos;
}

void XVideoPreviewWidget::preview(QPoint gpos) {
    on_setLoading(true);
    _videoPreview.extract(_file, _time);
    _currentPosition = QPoint(gpos);
    resize(70, 70);
    move(gpos - QPoint(70/2, 70));
    show();
}

void XVideoPreviewWidget::on_thumbExtract(QImage frame) {
    _label->setToolTip("");
    auto scaled = QPixmap::fromImage(frame.scaled(_thumbSize,_thumbSize,Qt::KeepAspectRatio));
    frame = QImage();
    resize(scaled.width(), scaled.height());
    move(_currentPosition - QPoint(scaled.width()/2, scaled.height()));
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
