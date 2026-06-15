SFMA (max) Funscript:
This is a format that puts all of the channels in a single funscript.
There a couple versions of this but XTP ONLY SUPPORTS > v2.0

MFS 
Multi funscript tracks works as follows:

You have your regular funscript for the stroke axis, no change here

After this you can pick another tracks from the devices available tracks
Create a funscript for it to match what the model does in that tracks or make it random.
Each funscript lives in the same directory.

The available tracks are:
sway
surge
pitch
roll
twist
vib
and more...

See the track column in channel setup in XTP settings for supported script names. You can even add your own if you wish.

Each script in the MFS set has a naming format:
<videoname>.<trackname>.funscript

The stroke tracks is always <videoname>.funscript for backwards compatibility.

Alternate scripts:
During metadata process funscript search, XTE will search for funscripts with the naming scheme:
<videoname><anythingYouWant>.funscript
These scripts will show up during playback for swapping out at runtime.



