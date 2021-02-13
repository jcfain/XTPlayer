#ifndef AUDIOSYNCFILTER_H
#define AUDIOSYNCFILTER_H

#include <QtAV/AudioFrame.h>
#include <QtAV/Filter.h>
#include <cmath>
#include "loghandler.h"


using namespace QtAV;
class AudioSyncFilter : public AudioFilter
{
    Q_OBJECT
signals:
    void levelChanged(int decibelL, int decibelR);
    void leftLevelChanged(int mLeft);
    void rightLevelChanged(int mRight);
public:
    AudioSyncFilter(QObject *parent = NULL);
protected:
    void process(Statistics* statistics, AudioFrame* frame = 0) Q_DECL_OVERRIDE;
private:
    int mLeft;
    int mRight;
};

#endif // AUDIOSYNCFILTER_H
