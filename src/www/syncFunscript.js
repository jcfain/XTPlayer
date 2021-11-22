
var funscript;
var funscripts;
//var isMediaFunscriptPlaying;
var ispaused = false;
var mediaPlaying = false;
var videoNode;
var lastMediaTime = 0;
var currentMediaTime = 0;
var currentTime = 0;
var _firstActionExecuted = false;
var remoteUserSettings;
var nextActionIndex = -1;
var lastActionIndex = -1;
var lastActionPos = -1;
var lastActionSpeed = -1;
var syncTimer;
onmessage = function(e) {
    //isMediaFunscriptPlaying = true;
    var data;
    if(typeof e.data === "string")
        data = JSON.parse(e.data);
    else
        data = e.data;
    switch(data["command"]) {
        case "setCurrentTime":
            currentMediaTime = Math.round(data["currentTime"] * 1000);
            break;
        case "setRemoteUserSettings":
            remoteUserSettings = data["remoteUserSettings"];
            break;
        case "startThread":
            console.log('Worker: Start thread');
            startThread(data);
            break;
        case "setPlayingState":
            ispaused = data["playingState"];
            break;
        case "setMediaPlayingState":
            mediaPlaying = data["playingState"];
            break;
        case "terminate":
            console.log('Worker: Terminate thread');
            //isMediaFunscriptPlaying = false;
            this.clearInterval(syncTimer)
            this.postMessage({"command": "end"});
            break;
    }
}

function startThread(data) {
    funscripts = data.funscripts;
    // var timer1 = 0;
    // var timer2 = Date.now();
    // while(isMediaFunscriptPlaying) {
    //     if (timer2 - timer1 >= 1)
    //     {
    //         timer1 = timer2;
    // console.log("Start: "+ Date.now());
    var timeTracker = 0;
    syncTimer = setInterval(function() {
        // var timer1 = Date.now();
        // console.log("Enter: "+ timer1);
            if(!ispaused && mediaPlaying)
            {
                // currentTime = currentMediaTime;
                // var hasRewind = lastMediaTime > currentTime;
                // if (currentTime > timeTracker + 100 || hasRewind)
                // {
                //     console.log("currentTime > timeTracker + 100 || hasRewind: "+ currentTime);
                //     lastMediaTime = currentTime;
                //     timeTracker = currentTime;
                // }
                // else
                // {
                //     timeTracker++;
                //     currentTime = timeTracker;
                //     console.log("else "+ currentTime);
                // }
                timeTracker++;
                currentTime = timeTracker;
                console.log("currentTime "+ currentTime);
                console.log("now "+ Date.now());
                var actions = [];
                for(var i = 0; i < funscripts.length; i++)
                {
                    var action = getPosition(funscripts[i], currentTime);
                    if(action)
                    {
                        actions.push({"channel": funscripts[i].channel, "action": action});
                    }
                }
                var tcode = funscriptToTCode(actions);
                if(tcode)
                    postMessage({"command": "sendTcode", "tcode": tcode});
            }
            
            // var timer2 = Date.now();
            // console.log("Exit: "+ timer2);
            // console.log("Took: "+ (timer2 - timer1) + "ms");
        }, 1);
    //     timer2 = Date.now();
    // }
}
function getPosition(funscript, millis)
{
    //var closestMillis = findClosest(millis, funscript.atList);
    //  console.log("currentTime: "+ currentTime);
    var closestMillis = funscript.atList.reduce(function(prev, curr) {
        return (Math.abs(curr - currentTime) < Math.abs(prev - currentTime) ? curr : prev);
      });
    // console.log("funscript[closestMillis]: "+ funscript[closestMillis]);
    // console.log("nextMillis: "+ nextMillis);
    if(closestMillis == -1) {
        console.log("return null !closestMillis || closestMillis == -1");
        return null;
    }
    nextActionIndex = funscript.atList.indexOf(closestMillis) + 1;
    // console.log("nextActionIndex: "+ nextActionIndex);
    if(nextActionIndex >= funscript.atList.length){
        console.log("return null nextActionIndex >= funscript.atList.length");
        return null;
    }
    var nextMillis = funscript.atList[nextActionIndex];
    //  console.log("nextMillis: "+ nextMillis);
    //  console.log("funscript[nextMillis]: "+ funscript[nextMillis]);
    // console.log("lastActionIndex: "+ lastActionIndex);
    if ((lastActionIndex != nextActionIndex && millis >= closestMillis) || lastActionIndex == -1)
    {
        var interval = lastActionIndex == -1 ? closestMillis : nextMillis - closestMillis;
        if(!_firstActionExecuted)
        {
            _firstActionExecuted = true;
            if(interval < 500)
                interval = 500;
        }
        var executionMillis = lastActionIndex == -1 ? closestMillis : nextMillis;
        var pos = funscript[executionMillis];
        nextAction = { "channel": funscript.channel, "at": executionMillis, "pos": pos, "speed": interval, "lastPos": lastActionPos, "lastSpeed": lastActionSpeed };
        //LogHandler::Debug("nextAction.speed: "+ QString::number(nextAction.speed));
        lastActionIndex = nextActionIndex;
        lastActionPos = funscript[executionMillis];
        lastActionSpeed = interval;
        // console.log("executionMillis: " + executionMillis);
        //  console.log("return action: " + funscript[executionMillis]);
        return nextAction;
    }
    //  console.log("return null");
    return null;
}

function findClosest(value, atList) {
    if(atList.length == 0)
        return -1;
      if(value < atList[0]) {
          return atList[0];
      }
      if(value > atList[atList.length-1]) {
          return atList[atList.length-1];
      }

      var lo = 0;
      var hi = atList.length - 1;

      while (lo <= hi) {
          var mid = (hi + lo) / 2;

          if (value < atList[mid]) {
              hi = mid - 1;
          } else if (value > atList[mid]) {
              lo = mid + 1;
          } else {
              return atList[mid];
          }
      }
      // lo == hi + 1
      return (atList[lo] - value) < (value - atList[hi]) ? atList[lo] : atList[hi];
  }

function funscriptToTCode(actions)
{
    for(var i=0;i<actions.length;i++)
    {
        var action = actions[i].action;
        var distance = action.pos >= action.lastPos ? action.pos - action.lastPos : action.lastPos - action.pos;
        // console.log("distance: "+distance);
        // console.log("action.pos: "+action.pos );
        // console.log("action.lastPos: "+action.lastPos );
        if(distance > 0)
        {
            var tcode = "";
            var position = action.pos;
            //console.log("Stroke pos: " + position + ", at: " + action.at);
            var speed = action.speed;
/*             if (FunscriptHandler::getInverted() || SettingsHandler::getChannelInverseChecked(TCodeChannelLookup::Stroke()))
            {
                position = XMath::reverseNumber(position, 0, 100);
            } */
            tcode += action.channel;
            tcode += calculateRange(action.channel, position).toString().padStart(4, '0');
            // LogHandler::Debug("Stroke tcode: "+ tcode);
            if (speed > 0)
            {
                tcode += "I";
                tcode += speed;
            }
            return tcode;
        }
    }
    return null;
}

function calculateRange(channel, rawValue) {
    var availableAxis = remoteUserSettings["availableAxis"];
    var channelObj = availableAxis[channel];
    if(channelObj) {
        var xMax = channelObj.userMax;
        var xMin = channelObj.userMin;
        var xMid = channelObj.userMid;
        return mapRange(rawValue, 50, 100, xMid, xMax);
    }
    return rawValue;
}

function channelLookup(channel) {
    switch(channel) {
        case "":
            return "L0";
        case "surge":
            return "L1";
        case "sway":
            return "L2";
        case "twist":
            return "R0";
        case "pitch":
            return "R2";
        case "roll":
            return "R1";
        case "vib":
            return "V0";
        case "suck":
            return "A0";
        case "suckPosition":
            return "A1";
        case "lube":
            return "A2";
    }
}

function mapRange(value, inStart, inEnd, outStart, outEnd) {
    slope = (outEnd - outStart) / (inEnd - inStart);
    return Math.round(outStart + slope * (value - inStart));
}