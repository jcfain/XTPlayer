#ifndef VIDEOHANDLER_H
#define VIDEOHANDLER_H
#include <QWidget>
#include <QtAV_Global.h>
#include <QtAV>
#include <QtAVWidgets>
#include <QHBoxLayout>

#include "lib/handler/settingshandler.h"
#include "lib/handler/loghandler.h"

using namespace QtAV;

class VideoHandler : public QWidget
{
    Q_OBJECT

signals:
    void doubleClicked(QMouseEvent* e);
    void keyPressed(QKeyEvent* k);
    void mouseEnter(QEvent* e);
public:
    VideoHandler(QWidget* parent = 0);
    virtual ~VideoHandler();

    AVPlayer* player;
    VideoOutput* videoRenderer;

    QHBoxLayout* layout;
private:
    void mouseDoubleClickEvent(QMouseEvent * e) override;
    void keyPressEvent(QKeyEvent * e) override;
    void enterEvent(QEvent * e) override;


};

#endif // VIDEOHANDLER_H
