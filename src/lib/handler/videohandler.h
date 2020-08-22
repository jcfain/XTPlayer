#ifndef VIDEOHANDLER_H
#define VIDEOHANDLER_H
#include <QWidget>
#include <QtAV>
#include <QtAVWidgets>
#include <QHBoxLayout>
#include <QOpenGLWidget>

#include "lib/handler/settingshandler.h"
#include "lib/handler/loghandler.h"

using namespace QtAV;

class VideoHandler : public QWidget
{
    Q_OBJECT

signals:
    void doubleClicked(QMouseEvent * e);
public:
    VideoHandler(QWidget* parent = 0);
    virtual ~VideoHandler();

    AVPlayer* player;
    VideoOutput* videoRenderer;

    QHBoxLayout* layout;
private:
    void mouseDoubleClickEvent(QMouseEvent * e) override;

};

#endif // VIDEOHANDLER_H
