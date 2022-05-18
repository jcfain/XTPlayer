#ifndef XVIDEOSURFACE_H
#define XVIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <QImage>
#include <QRect>
#include <QVideoFrame>
#include <QLabel>
#include <QtWidgets>
#include <qabstractvideosurface.h>
#include <qvideosurfaceformat.h>

class XVideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
signals:
    void fnSurfaceStopped(QPixmap pix);
public:
    XVideoSurface(QObject *parent = 0);
    void stop();
private slots:
    void fnClearPixmap();
private:
    QImage::Format imageFormat;
    QPixmap imageCaptured;
    QRect targetRect;
    QRect sourceRect;
    QVideoFrame currentFrame;

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
    bool isFormatSupported(const QVideoSurfaceFormat &format) const;

    bool start(const QVideoSurfaceFormat &format);


    bool present(const QVideoFrame &frame);

    QRect videoRect() const { return targetRect; }
};

#endif // XVIDEOSURFACE_H
