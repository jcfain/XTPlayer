#include "xvideopreviewwidget.h"

XVideoPreviewWidget::XVideoPreviewWidget(QWidget* parent) : QWidget(parent)
{
    resize(176, 100);
    _layout = new QGridLayout(this);
    _label = new QLabel(this);
    _label->setScaledContents(true);
    setLayout(_layout);
    _layout->addWidget(_label);

    _videoLoadingMovie = new QMovie("://images/Eclipse-1s-loading-200px.gif");
    _videoLoadingMovie->setProperty("cssClass", "mediaLoadingSpinner");
    _videoLoadingLabel = new QLabel(this);
    _videoLoadingLabel->setMovie(_videoLoadingMovie);
    _videoLoadingLabel->setAttribute(Qt::WA_TransparentForMouseEvents );
    _videoLoadingLabel->setMaximumSize(100,100);
    //_videoLoadingLabel->setStyleSheet("* {background: ffffff}");
    _videoLoadingLabel->setProperty("cssClass", "mediaLoadingSpinner");
    _videoLoadingLabel->setAlignment(Qt::AlignCenter);
    _videoLoadingLabel->setScaledContents(true);
    _layout->addWidget(_videoLoadingLabel);

    connect(&_videoPreview, &XVideoPreview::frameExtracted, this, &XVideoPreviewWidget::on_thumbExtract);
}

void XVideoPreviewWidget::setFile(QString path) {
    _file = path;
}

void XVideoPreviewWidget::setTimestamp(qint64 pos) {
    _time = pos;
}

void XVideoPreviewWidget::preview() {
    on_setLoading(true);
    _videoPreview.extract(_file, _time);
}

void XVideoPreviewWidget::on_thumbExtract(QPixmap frame) {
    _label->setToolTip("");
    _label->setPixmap(frame);
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
