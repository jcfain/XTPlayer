/*
#ifndef XMEDIAPLAYER_H
#define XMEDIAPLAYER_H
#include <QMediaPlayer>

class XMediaPlayer : public QMediaPlayer
{
    Q_OBJECT
public:
    XMediaPlayer(QObject *parent = nullptr) : QMediaPlayer(parent)
    {
        //setFixedSize(20, 20);
    }
    //virtual ~XMediaPlayer() override = default;
signals:
    //void error();
protected:
    //void errorEvent(){
       // emit error();
    //}
public:
    virtual QMultimedia::AvailabilityStatus availability() const override
    {
        return QMediaPlayer::availability();
    }
};

#endif // XMEDIAPLAYER_H
*/
