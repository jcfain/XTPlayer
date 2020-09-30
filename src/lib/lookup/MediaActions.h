#ifndef MEDIAACTIONS_H
#define MEDIAACTIONS_H

#include <QString>
#include <QMap>
struct MediaActions
{
    const QString TogglePause = "TogglePause";
    const QString Next = "Next";
    const QString Back = "Back";
    const QString FullScreen = "FullScreen";
    const QString VolumeUp = "VolumeUp";
    const QString VolumeDown = "VolumeDown";
    const QString Mute = "Mute";
    const QString Stop = "Stop";
    const QString Loop = "Loop";
    const QString Rewind = "Rewind";
    const QString FastForward = "Fast forward";
    const QString TCodeSpeedUp = "TCodeSpeedUp";
    const QString TCodeSpeedDown = "TCodeSpeedDown";
    const QString IncreaseXRange = "IncreaseXRange";
    const QString DecreaseXRange = "DecreaseXRange";
    const QString IncreaseXUpperRange = "IncreaseXUpperRange";
    const QString DecreaseXUpperRange = "DecreaseXUpperRange";
    const QString IncreaseXLowerRange = "IncreaseXLowerRange";
    const QString DecreaseXLowerRange = "DecreaseXLowerRange";
    const QString ResetLiveXRange = "ResetLiveXRange";
    const QMap<QString, QString> Values {
        {TogglePause, "Toggle pause"},
        {Next, "Next video"},
        {Back, "Previous video"},
        {FullScreen, "Toggle fullscreen"},
        {VolumeUp, "Volume up"},
        {VolumeDown, "Volume down"},
        {Mute, "Toggle mute"},
        {Stop, "Stop video"},
        {Loop, "Toggle loop A/B/off"},
        {Rewind, "Rewind"},
        {FastForward, "Fast forward"},
        {TCodeSpeedUp, "TCode speed up"},
        {TCodeSpeedDown, "TCode speed down"},
        {IncreaseXRange, "Increase X range"},
        {DecreaseXRange, "Decrease X range"},
        {IncreaseXUpperRange, "Increase X upper range"},
        {DecreaseXUpperRange, "Decrease X upper range"},
        {IncreaseXLowerRange, "Increase X lower range"},
        {DecreaseXLowerRange, "Decrease X lower range"},
        {ResetLiveXRange, "Reset X range"}
    };
};
#endif // MEDIAACTIONS_H
