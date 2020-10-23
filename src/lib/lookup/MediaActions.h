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
    const QString ToggleAxisMultiplier = "ToggleAxisMiltiplier";
    const QString ToggleFunscriptInvert = "ToggleFunscriptInvert";
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
        {TCodeSpeedUp, "Gamepad speed up"},
        {TCodeSpeedDown, "Gamepad speed down"},
        {IncreaseXRange, "Increase stroke range"},
        {DecreaseXRange, "Decrease stroke range"},
        {IncreaseXUpperRange, "Increase stroke upper range"},
        {DecreaseXUpperRange, "Decrease stroke upper range"},
        {IncreaseXLowerRange, "Increase stroke lower range"},
        {DecreaseXLowerRange, "Decrease stroke lower range"},
        {ResetLiveXRange, "Reset stroke range"},
        {ToggleAxisMultiplier, "Toggle axis multiplier"},
        {ToggleFunscriptInvert, "Toggle funscript inversion"}
    };
};
#endif // MEDIAACTIONS_H
