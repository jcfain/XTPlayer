Multi funscript tracks works as follows:

You have your regular funscript for the X axis (up/down) no change here

After this you can pick another axis from the devices available axis’
Create a funscript for it to match what the model does in that axis or make it random.
Each funscript lives in the same directory. (I hope to have a package system soon)

The available axis’ are:
sway
surge
pitch
roll
twist

Note: the OSR2+ only has pitch, roll and maybe twist. OSR2 only has roll.
sway and surge are reserved for future devices.

Each script has a naming format if outside a zip file
<videoname>.<axisname>.funscript
if in a zip file the name scheme is as follows:
<videoname>.zip / <videoname>.<axisname>.funscript

The stroke axis is always <videoname>.funscript for backwards compatibility

These axis all are similar to the x axis where 50 = home position
with rotate axis 49-0 rotate counter clockwise and 51-100 rotate clockwise.
(it seems pitch maybe the inverse of this but I need to confirm)

Notes for playbacl:
If there is an existing <videoname>.funscript (or you select play with funscript) or <videoname>.<axisname>.funscript outside of the zip file
This will override the contents of the zip file.