#include "xvideopreviewwidget.h"

XVideoPreviewWidget::XVideoPreviewWidget(QWidget* parent) : QWidget(parent)
{
    _layout = new QGridLayout(this);
    _label = new QLabel(this);
    _label->setScaledContents(true);
    setLayout(_layout);
    _layout->addWidget(_label);
    connect(&_videoPreview, &XVideoPreview::frameExtracted, this, &XVideoPreviewWidget::on_thumbExtract);
}

void XVideoPreviewWidget::setFile(QString path) {
    _file = path;
}

void XVideoPreviewWidget::setTimestamp(qint64 pos) {
    _time = pos;
}

void XVideoPreviewWidget::preview() {
    _videoPreview.extract(_file, _time);
}

void XVideoPreviewWidget::on_thumbExtract(QPixmap frame) {
    _label->setToolTip("");
    _label->setPixmap(frame);
    _label->update();
}

void XVideoPreviewWidget::on_thumbExtractionError(QString error) {
    _label->setToolTip(error);
    emit thumbExtractionError(error);
}

void XVideoPreviewWidget::closeEvent(QCloseEvent *event) {
  QWidget::closeEvent(event);
}
