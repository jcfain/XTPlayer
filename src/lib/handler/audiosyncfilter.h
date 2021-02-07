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
    void leftLevelChanged(int value);
    void rightLevelChanged(int value);
public:
    AudioSyncFilter(QObject *parent = NULL);
protected:
    void process(Statistics* statistics, AudioFrame* frame = 0) Q_DECL_OVERRIDE;
private:
    int mLeft;
    int mRight;
};

#endif // AUDIOSYNCFILTER_H
