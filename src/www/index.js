
var DeviceType =
{
    Serial: 0,
    Network: 1,
    Deo: 2,
    Whirligig: 3,
    Gamepad: 4,
    XTPWeb: 5,
	None: 6
};

var ConnectionStatus =
{
    Connected: 0,
    Disconnected: 1,
    Connecting: 2,
    Error: 3
};

var ChannelType = 
{
    None: 0,
    Range: 1,
    Switch: 2,
    HalfRange: 3
}

var AxisDimension =
{
    None: 0,
    Heave: 1,
    Surge: 2,
    Sway: 3,
    Pitch: 4,
    Roll: 5,
    Yaw: 6
};

var wsUri;
var websocket = null;

var userAgent;
var userAgentIsDeo = false;
var remoteUserSettings;
var mediaListObj = [];
var playingmediaItem;
var playingmediaItemNode;
var thumbsContainerNode;
var sortByGlobal = "nameAsc";
var showGlobal = "All";
var settingsNode;
var thumbSizeGlobal = 0;
var videoNode;
var videoSourceNode;
var externalStreaming;
var resizeObserver;
var webSocket;
var deviceAddress;
var funscriptChannels = [];
//var loadedFunscripts;
var currentChannelIndex = 0;
var connectionStatus = ConnectionStatus.Disconnected;
var deviceConnectionStatusInterval;
var deviceConnectionStatusRetryButtonNodes;
var deviceConnectionStatusRetryButtonImageNodes;
//var funscriptSyncWorker;
//var useDeoWeb;
//var deoVideoNode;
//var deoSourceNode;


document.addEventListener("DOMContentLoaded", function() {
  loadPage();
});
function loadPage()
{
	getBrowserInformation();
	userAgentIsDeo = userAgent.indexOf("Deo VR") != -1;
	setDeoStyles(userAgentIsDeo);
	settingsNode = document.getElementById("settingsModal");
	thumbsContainerNode = document.getElementById("thumbsContainer");

	sortByGlobal = JSON.parse(window.localStorage.getItem("sortBy"));
	showGlobal = JSON.parse(window.localStorage.getItem("show"));
	thumbSizeGlobal = JSON.parse(window.localStorage.getItem("thumbSize"));
/* 	if(!thumbSizeGlobal && window.devicePixelRatio == 2.75) {
		thumbSizeGlobal = 400;
	} */
	var volume = JSON.parse(window.localStorage.getItem("volume"));
	externalStreaming = JSON.parse(window.localStorage.getItem("externalStreaming"));
	//deviceAddress =  JSON.parse(window.localStorage.getItem("webSocketAddress"));
	// if(!deviceAddress)
	// 	deviceAddress = "tcode.local";
		
	//document.getElementById("webSocketAddress").value = deviceAddress;

	videoNode = document.getElementById("videoPlayer");
	videoSourceNode = document.getElementById("videoSource");
	videoNode.addEventListener("timeupdate", onVideoTimeUpdate); 
	videoNode.addEventListener("loadeddata", onVideoLoad); 
	videoNode.addEventListener("play", onVideoPlay); 
	videoNode.addEventListener("playing", onVideoPlaying); 
	videoNode.addEventListener("stalled", onVideoStall); 
	videoNode.addEventListener("waiting", onVideoStall); 
	videoNode.addEventListener("pause", onVideoPause); 
	videoNode.addEventListener("volumechange", onVolumeChange); 
	videoNode.addEventListener("ended", onVideoEnd); 
	videoNode.volume = volume ? volume : 0.5;
	// Fires on load?
	// videoSourceNode.addEventListener('error', function(event) { 
	// 	alert("There was an issue loading media.");
	// }, true);
	// videoNode.addEventListener('error', function(event) { 
	// 	alert("There was an issue loading media.");
	// }, true);

	deviceConnectionStatusRetryButtonNodes = document.getElementsByName("deviceStatusRetryButton");
	deviceConnectionStatusRetryButtonImageNodes = document.getElementsByName("connectionStatusIconImage");
	toggleExternalStreaming(externalStreaming, false);
	
/* 	
	deoVideoNode = document.getElementById("deoVideoPlayer");
	deoSourceNode = document.getElementById("deoVideoSource");
	deoVideoNode.addEventListener("end", onVideoStop); 
	useDeoWeb = JSON.parse(window.localStorage.getItem("useDeoWeb"));

	if(useDeoWeb) {
		toggleUseDeo(useDeoWeb, false);
		new ResizeObserver(onResizeDeo).observe(deoVideoNode)
	} 
*/
	getServerSettings();
	
}

debugMode = true;
function debug(message) {
	if(debugMode)
		console.log(message);
}

function sendMessageXTP(command, message) {
	if(websocket) {
		var obj;
		if(!message) {
			obj = {"command": command}
		} else {
			obj = {"command": command, "message": message}
		}
		websocket.send(JSON.stringify(obj));
	}
}

function onSyncDeviceConnectionChange(input, device) {
	sendMessageXTP("connectSyncDevice", {deviceType: device, enabled: input.checked});
}

function tcodeDeviceConnectRetry() {
	sendMessageXTP("connectTCodeDevice");
}

function userGetDeviceConnectionStatus() {
	sendMessageXTP("connectionStatus");
} 

function sendTCode(tcode) {
	sendMessageXTP("tcode", tcode);
}

function stopWebSocket() {
	if (websocket)
		websocket.close();
}

function onResizeVideo() {
	thumbsContainerNode.style.maxHeight = "calc(100vh - "+ (+videoNode.offsetHeight + 165) + "px)";
} 
function getBrowserInformation() {
	var nAgt = navigator.userAgent;
	var browserName;
	var fullVersion; 
	var majorVersion;
	var nameOffset,verOffset,ix;
	
	// In Opera, the true version is after "Opera" or after "Version"
	if ((verOffset=nAgt.indexOf("Opera"))!=-1) {
	 browserName = "Opera";
	 fullVersion = nAgt.substring(verOffset+6);
	 if ((verOffset=nAgt.indexOf("Version"))!=-1) 
	   fullVersion = nAgt.substring(verOffset+8);
	}
	// In MSIE, the true version is after "MSIE" in userAgent
	else if ((verOffset=nAgt.indexOf("MSIE"))!=-1) {
	 browserName = "Microsoft Internet Explorer";
	 fullVersion = nAgt.substring(verOffset+5);
	}
	// In Chrome, the true version is after "Chrome" 
	else if ((verOffset=nAgt.indexOf("Chrome"))!=-1) {
	 browserName = "Chrome";
	 fullVersion = nAgt.substring(verOffset+7);
	}
	// In Safari, the true version is after "Safari" or after "Version" 
	else if ((verOffset=nAgt.indexOf("Safari"))!=-1) {
	 browserName = "Safari";
	 fullVersion = nAgt.substring(verOffset+7);
	 if ((verOffset=nAgt.indexOf("Version"))!=-1) 
	   fullVersion = nAgt.substring(verOffset+8);
	}
	// In Firefox, the true version is after "Firefox" 
	else if ((verOffset=nAgt.indexOf("Firefox"))!=-1) {
	 browserName = "Firefox";
	 fullVersion = nAgt.substring(verOffset+8);
	}
	// In most other browsers, "name/version" is at the end of userAgent 
	else if ( (nameOffset=nAgt.lastIndexOf(' ')+1) < 
			  (verOffset=nAgt.lastIndexOf('/')) ) 
	{
	 browserName = nAgt.substring(nameOffset,verOffset);
	 fullVersion = nAgt.substring(verOffset+1);
	 if (browserName.toLowerCase()==browserName.toUpperCase()) {
	  browserName = navigator.appName;
	 }
	}
	// trim the fullVersion string at semicolon/space if present
	if ((ix=fullVersion.indexOf(";"))!=-1)
	   fullVersion=fullVersion.substring(0,ix);
	if ((ix=fullVersion.indexOf(" "))!=-1)
	   fullVersion=fullVersion.substring(0,ix);
	
	majorVersion = parseInt(''+fullVersion,10);
	if (isNaN(majorVersion)) {
	 fullVersion  = ''+parseFloat(navigator.appVersion); 
	 majorVersion = parseInt(navigator.appVersion,10);
	}
	
	var divnode = document.createElement("div"); 
	divnode.innerHTML = ''
	 +'Browser name  = '+browserName+'<br>'
	 +'Full version  = '+fullVersion+'<br>'
	 +'Major version = '+majorVersion+'<br>'
	 +'navigator.userAgent = '+navigator.userAgent+'<br>';
	 userAgent = navigator.userAgent;
	document.getElementById("browserInfoTab").appendChild(divnode)
}
function setDeoStyles(isDeo) {
	if(isDeo) {
		var checkboxes = document.querySelectorAll("input[type='checkbox']");
		for(var i=0;i<checkboxes.length;i++) {
			checkboxes[i].classList.remove("styled-checkbox")
		}
	} else {
		var checkboxes = document.getElementsByClassName("input[type='checkbox']");
		for(var i=0;i<checkboxes.length;i++) {
			checkboxes[i].classList.add("styled-checkbox");
		}
	}
}
/* 
function onResizeDeo() {
	if(useDeoWeb) {
		thumbsContainerNode.style.maxHeight = "calc(100vh - "+ (+deoVideoNode.offsetHeight + 120) + "px)";
	}
} 
*/

function getServerSettings() {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', "/settings", true);
	xhr.responseType = 'json';
	xhr.onload = function() {
	  var status = xhr.status;
	  if (status === 200) {
		remoteUserSettings = xhr.response;
		funscriptChannels = Object.keys(remoteUserSettings["availableAxis"])
			.map(function (k) {
				return remoteUserSettings["availableAxis"][k]["channel"];
			});
		remoteUserSettings.availableAxisArray = Object.keys(remoteUserSettings["availableAxis"])
			.map(function (k) {
				return remoteUserSettings["availableAxis"][k];
			});
		funscriptChannels.sort();
		initWebSocket();
		updateSettingsUI();
		getServerLibrary();
	  } else {
		alert("Error getting settings");
	  }
	};
	xhr.send();
}

function updateSettingsUI() {
	setupSliders();
	setupConnectionsTab();

	document.getElementById("tabLocalTab").onclick();
}

function getServerLibrary() {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', "/media", true);
	xhr.responseType = 'json';
	xhr.onload = function() {
	  var status = xhr.status;
	  if (status === 200) {
		onMediaLoad(null, xhr.response);
	  } else {
		onMediaLoad(status, xhr.response);
	  }
	};
	xhr.onerror = function() {
		alert("Error getting media");
	};
	xhr.send();
}

function initWebSocket() {
	try {
		wsUri = "ws://" + window.location.host + ":8080";
		if (typeof MozWebSocket == 'function')
			WebSocket = MozWebSocket;
		if ( websocket && websocket.readyState == 1 )
			websocket.close();
		websocket = new WebSocket( wsUri );
		websocket.onopen = function (evt) {
			debug("CONNECTED");
			userGetDeviceConnectionStatus();
			sendMessageXTP("syncConnectionStatus");
		};
		websocket.onclose = function (evt) {
			debug("DISCONNECTED");
		};
		websocket.onmessage = function (evt) {
			wsCallBackFunction(evt);
		};
		websocket.onerror = function (evt) {
			debug('ERROR: ' + evt.data);
		};
	} catch (exception) {
		debug('ERROR: ' + exception);
	}
}

function wsCallBackFunction(evt) {
	console.log( "Message received :", evt.data );
	var data = JSON.parse(evt.data);
	switch(data["command"]) {
		case "deviceStatus":
			var status = data["message"];
			var deviceType = status["deviceType"];
			setConnectionStatus(status["status"], status["message"])
			break;
		case "syncDeviceStatus":
			var status = data["message"];
			var deviceType = status["deviceType"];
			updateSyncStatus(deviceType, status["status"], status["message"])
			break;
		case "connectionClosed":
			alert("Looks like XTP was shut down.\nRestart XTP and refresh the page to reconnect.");
			break;
		case "mediaLoaded":
			getServerLibrary();
			break;
	}
}

function updateSyncStatus(deviceType, status, message) {
	// check &#x2714;
	// x &#x2718;
	// triangle ! &#x26A0;
	var statusImage = ""
	var statusColor = "";
	switch (status) {
		case ConnectionStatus.Connected:
			statusImage = "://images/icons/check-mark-black.png";
			statusColor = "chartreuse";
			break;
		case ConnectionStatus.Connecting:
			statusImage = "://images/icons/reload.svg";
			statusColor = "yellow";
			break;
		case ConnectionStatus.Disconnected:
			statusImage = "://images/icons/x.png";
			statusColor = "crimson";
			break;
		case ConnectionStatus.Error:
			statusImage = "://images/icons/error-black.png";
			statusColor = "red";
			break;
	}
	var deoVRStatusImage = document.getElementById("deoVRStatus")
	var whirligigStatusImage = document.getElementById("whirligigStatus")
	var xtpWebStatusImage = document.getElementById("xtpWebStatus")
	var gamepadStatusImage = document.getElementById("gamepadStatus")
	deoVRStatusImage.src = "";
	whirligigStatusImage.src = "";
	xtpWebStatusImage.src = "";
	gamepadStatusImage.src = "";
	switch(deviceType) {
		case DeviceType.Deo:
			deoVRStatusImage.src = statusImage;
			deoVRStatusImage.style.backgroundColor = statusColor;
			deoVRStatusImage.title = message;
			break;
		case DeviceType.Whirligig:
			whirligigStatusImage.src = statusImage;
			whirligigStatusImage.style.backgroundColor = statusColor;
			whirligigStatusImage.title = message;
			break;
		case DeviceType.XTPWeb:
			xtpWebStatusImage.src = statusImage;
			xtpWebStatusImage.style.backgroundColor = statusColor;
			xtpWebStatusImage.title = message;
			break;
		case DeviceType.Gamepad:
			gamepadStatusImage.src = statusImage;
			gamepadStatusImage.style.backgroundColor = statusColor;
			gamepadStatusImage.title = message;
			break;

	}
}
function setConnectionStatus(status, message) {
	for(var i=0;i<deviceConnectionStatusRetryButtonNodes.length;i++) {
		var deviceConnectionStatusRetryButtonNode = deviceConnectionStatusRetryButtonNodes[i];
		var deviceConnectionStatusRetryButtonImageNode = deviceConnectionStatusRetryButtonImageNodes[i];
		deviceConnectionStatusRetryButtonNode.title = "TCode status: " + message;
		deviceConnectionStatusRetryButtonNode.style.cursor = "";
		deviceConnectionStatusRetryButtonNode.disabled = true;
		var tcodeDeviceSettingsLink = document.getElementById("tcodeDeviceSettingsLink");
		tcodeDeviceSettingsLink.hidden = true;
		switch(status) {
			case ConnectionStatus.Disconnected:
				deviceConnectionStatusRetryButtonNode.style.backgroundColor = "crimson";
				deviceConnectionStatusRetryButtonNode.style.cursor = "pointer";
				deviceConnectionStatusRetryButtonNode.disabled = false;
				deviceConnectionStatusRetryButtonImageNode.src = "://images/icons/x.png";
				deviceConnectionStatusRetryButtonNode.title += ": Check your devices connection and click this to retry"
				break;
			case ConnectionStatus.Connecting:
				deviceConnectionStatusRetryButtonNode.style.backgroundColor = "yellow";
				deviceConnectionStatusRetryButtonImageNode.src = "://images/icons/reload.svg";
				break;
			case ConnectionStatus.Connected:
				deviceConnectionStatusRetryButtonNode.style.backgroundColor = "chartreuse";
				deviceConnectionStatusRetryButtonImageNode.src = "://images/icons/check-mark-black.png";
				if(remoteUserSettings.connection.selectedTCodeDevice == DeviceType.Network) {
					tcodeDeviceSettingsLink.href = "http://"+remoteUserSettings.connection.networkAddress;
					tcodeDeviceSettingsLink.hidden = false;
				}
				break;
			case ConnectionStatus.Error:
				deviceConnectionStatusRetryButtonNode.style.backgroundColor = "red";
				deviceConnectionStatusRetryButtonNode.style.cursor = "pointer";
				deviceConnectionStatusRetryButtonNode.disabled = false;
				deviceConnectionStatusRetryButtonImageNode.src = "://images/icons/error-black.png";
				deviceConnectionStatusRetryButtonNode.title += ": Check your devices connection and click this to retry"
				break;

		}
	}
}
/* function getDeviceConnectionStatus() {
	if(deviceConnectionStatusInterval)
		clearTimeout(deviceConnectionStatusInterval);
	var xhr = new XMLHttpRequest();
	xhr.open('GET', "/settings/deviceConnectionStatus", true);
	xhr.responseType = 'json';
	xhr.onload = function() {
	  var status = xhr.status;
	  if (status === 200) {
		var connectionStatusEvent = xhr.response;
		connectionStatus = connectionStatusEvent.status;
		setConnectionStatus(connectionStatus, connectionStatusEvent.message);
		pollDeviceConnectionStatus(30000);
		if(connectionStatus == ConnectionStatus.Connecting) {
			pollDeviceConnectionStatus(1000);
		}
	  } else {
		alert("Http Error getting device connection status: "+ status);
		if(deviceConnectionStatusInterval)
			clearTimeout(deviceConnectionStatusInterval);
			setConnectionStatus(ConnectionStatus.Error, "Http Error getting device connection status: "+ status);
	  }
	};
	xhr.onerror = function() {
		var status = xhr.status;
		alert("Error getting device connection status: "+ status);
		if(deviceConnectionStatusInterval)
			clearTimeout(deviceConnectionStatusInterval);
			setConnectionStatus(ConnectionStatus.Error, "Error getting device connection status: "+ status);
	};
	xhr.send();
} */

/* function pollDeviceConnectionStatus(pollInterval) {
	if(deviceConnectionStatusInterval) {
		clearTimeout(deviceConnectionStatusInterval);
		deviceConnectionStatusInterval = null;
	}
	deviceConnectionStatusInterval = setTimeout(function () {
		getDeviceConnectionStatus();
	}, pollInterval);
} */


function getMediaFunscripts(path, isMFS) {
	var channel = funscriptChannels[currentChannelIndex];
	var trackName = remoteUserSettings["availableAxis"][channel]["trackName"];
	var xhr = new XMLHttpRequest();
	var channelPath = trackName === "" ? trackName : "."+trackName;
	xhr.open('GET', path + channelPath + ".funscript", true);
	xhr.responseType = 'json';
	xhr.onload = function() {
		var status = xhr.status;
		if (status === 200) {
			loadedFunscripts.push({"channel": channel, "atList": []});
			var index = loadedFunscripts.length - 1;
			var items = xhr.response;
			for(var i=0; i<items.actions.length;i++)
			{
				var item = items.actions[i];
				var at = item["at"];
				loadedFunscripts[index][at] = item["pos"];
				loadedFunscripts[index].atList.push(at);
			}
		}
		currentChannelIndex++;
		if(currentChannelIndex < funscriptChannels.length)
		{
			getMediaFunscripts(path, isMFS);
		} 
		else 
		{
			currentChannelIndex = 0;
			videoNode.play();
			console.log("Funscripts load finish");
		}
	};
	xhr.send();
}

function postServerSettings() {
	var xhr = new XMLHttpRequest();
	xhr.open('POST', "/settings", true);
	xhr.setRequestHeader('Content-Type', 'application/json');
	xhr.onreadystatechange = function() {
	  if (xhr.readyState === 4) {
		var status = xhr.status;
		if (status !== 200) 
			alert('Error saving server settings: '+status)
		else
			alert('XTP settings saved!')
	  }
	}
	xhr.onerror = function() {
		var status = xhr.status;
		alert('Error saving server settings: '+status)
	};
	xhr.send(JSON.stringify(remoteUserSettings));
}

function postMediaState(mediaState) {
	var xhr = new XMLHttpRequest();
	xhr.open("POST", "/xtpweb", true);
	xhr.setRequestHeader('Content-Type', 'application/json');
	xhr.send(JSON.stringify(mediaState));
	xhr.oneload = function() {
		var status = xhr.status;
		if (status !== 200) 
			console.log('Error sending mediastate: '+status)
	};
	xhr.onerror = function() {
		var status = xhr.status;
		console.log('Error sending mediastate: '+status)
	};
}

function onMediaLoad(err, mediaList)
{
  if (err !== null) {
    alert('Whoops!: ' + err);
  } else {
	mediaListObj = [];
	for(var i=0; i<mediaList.length;i++)
	{
		var obj = {
			id: mediaList[i]["name"].replace(/^[^a-z]+|[^\w:.-]+/gi, "")+"item"+i,
			name: mediaList[i]["name"],
			displayName: mediaList[i]["name"],
			thumbSize: mediaList[i]["thumbSize"],
			path: mediaList[i]["path"],
			relativePath: mediaList[i]["relativePath"],
			thumb: mediaList[i]["thumb"],
			relativeThumb: mediaList[i]["relativeThumb"],
			modifiedDate: new Date(mediaList[i]["modifiedDate"]),
			isStereoscopic: mediaList[i]["isStereoscopic"],
			isMFS: mediaList[i]["isMFS"],
			hasScript: mediaList[i]["hasScript"],
			scriptNoExtensionRelativePath: mediaList[i]["scriptNoExtensionRelativePath"],
			loaded: false,
			playing: false
		}
		if(obj.isMFS)
			obj.displayName = "(MFS) " + obj.name;
		mediaListObj.push(obj);
	}
/* 	
	if(useDeoWeb && mediaListObj.length > 0)
		loadVideo(mediaListObj[0]); 
*/
	updateMediaUI();
  }
}

function updateMediaUI() {
	setThumbSize(thumbSizeGlobal, false);
	sort(sortByGlobal, false);
	loadMedia(show(showGlobal, false))
}

function loadMedia(mediaList) {
	var medialistNode = document.getElementById("mediaList");
	removeAllChildNodes(medialistNode);
	var noMediaElement = document.getElementById("noMedia");
	if(!mediaList || mediaList.length == 0)
	{ 
		noMediaElement.hidden = false;
		return;
	}
	noMediaElement.hidden = true;
	
	var createClickHandler = function(obj) { 
		return function() { 
			//loadVideo(obj); 
			playVideo(obj); 
		} 
	};
	
	var textHeight = 0
	var width = 0;
	var height = 0;
	var fontSize = 0;
	for(var i=0; i<mediaList.length;i++)
	{
		var obj = mediaList[i];
		if(!thumbSizeGlobal) {
			setThumbSize(obj.thumbSize, true);
			setThumbSize(obj.thumbSize, false);
		}
		if(!textHeight)
		{
			textHeight = (thumbSizeGlobal * 0.25);
			width = thumbSizeGlobal + (thumbSizeGlobal * 0.15) + "px";
			height = thumbSizeGlobal + textHeight + "px";
			fontSize = (textHeight * 0.3) + "px";
		}
		var divnode = document.createElement("div"); 
		divnode.id = obj.id
		divnode.className += "media-item"
		divnode.style.width = width;
		divnode.style.height = height;
		divnode.title = obj.name;
		var anode = document.createElement("a"); 
		anode.className += "media-link"
		if(obj.isMFS) {
			divnode.className += " media-item-mfs"
		}
		if(!obj.hasScript) {
			divnode.className += " media-item-noscript"
		}
		//anode.style.width = width;
		//anode.style.height = height;
		anode.onclick = createClickHandler(obj);
		var image = document.createElement("img"); 
		image.src = "/thumb/" + obj.relativeThumb;
		image.style.maxWidth = thumbSizeGlobal + "px";
		image.style.maxHeight = thumbSizeGlobal + "px";
		image.id = obj.id;
		var namenode = document.createElement("div");
		namenode.innerText = obj.displayName;
		namenode.className += "name"
		namenode.style.width = width;
		namenode.style.height = textHeight + "px";
		namenode.style.fontSize = fontSize;
		
		divnode.appendChild(anode);
		anode.appendChild(image);
		anode.appendChild(namenode);
		medialistNode.appendChild(divnode);
		if(playingmediaItem && playingmediaItem.id === obj.id)
			setPlayingMediaItem(obj);
	}
}

function removeAllChildNodes(parent) {
    while (parent.firstChild) {
        parent.removeChild(parent.firstChild);
    }
}

function sort(value, userClick) {
	if(!value)
		value = "nameAsc";
	switch(value) {
		case "dateDesc":
			mediaListObj.sort(function(a,b){
			  return new Date(b.modifiedDate) - new Date(a.modifiedDate);
			});
		break;
		case "dateAsc":
			mediaListObj.sort(function(a,b){
			  return new Date(a.modifiedDate) - new Date(b.modifiedDate);
			});
		break;
		case "nameAsc":
			mediaListObj.sort(function(a,b){
			  var nameA = a.displayName.toUpperCase(); // ignore upper and lowercase
			  var nameB = b.displayName.toUpperCase(); // ignore upper and lowercase
			  if (nameA < nameB) {
				return -1;
			  }
			  if (nameA > nameB) {
				return 1;
			  }
			});
		break;
		case "nameDesc":
			mediaListObj.sort(function(a,b){
			  var nameA = a.displayName.toUpperCase(); // ignore upper and lowercase
			  var nameB = b.displayName.toUpperCase(); // ignore upper and lowercase
			  if (nameB < nameA) {
				return -1;
			  }
			  if (nameB > nameA) {
				return 1;
			  }
			});
		break;
	}
	if(!userClick)
		document.getElementById("sortBy").value = value;
	else {
		window.localStorage.setItem("sortBy", JSON.stringify(value));
		sortByGlobal = value;
	}
}

function show(value, userClick) {
	if(!value)
		value = "All";
	filteredMedia = [];
	switch(value) {
		case "All":
			filteredMedia = mediaListObj;
		break;
		case "3DOnly":
			filteredMedia = mediaListObj.filter(x => x.isStereoscopic);
		break;
		case "2DAndAudioOnly":
			filteredMedia = mediaListObj.filter(x => !x.isStereoscopic);
		break;
	}
	if(!userClick)
		document.getElementById("show").value = value;
	else {
		window.localStorage.setItem("show", JSON.stringify(value));
		showGlobal = value;
	}
	return filteredMedia;
}

/* 
function onClickUseDeoWebCheckbox(checkbox)
{
	toggleUseDeo(checkbox.checked, true);
}

function toggleUseDeo(value, userClicked)
{
	if(userClicked) {
		useDeoWeb = value;
		window.localStorage.setItem("useDeoWeb", JSON.stringify(useDeoWeb));
	}
	else
		document.getElementById("useDeoWebCheckbox").checked = value;
	if(!useDeoWeb)
	{
		deoVideoNode.style.display = "none";
		thumbsContainerNode.style.maxHeight = "";
	} else {
		deoVideoNode.style.display = "block";
		onResizeDeo();
	}
}

function loadVideo(obj) {
	if(useDeoWeb) {
		deoVideoNode.setAttribute("format", obj.isStereoscopic ? "LR" : "mono");
		deoSourceNode.setAttribute("src", "/video/" + obj.relativePath);
		deoVideoNode.setAttribute("title", obj.name);
		deoVideoNode.setAttribute("cover-image", "/thumb/" + obj.relativeThumb);
		
		if(!DEO.isStarted(deoVideoNode))
			DEO.setVolume(deoVideoNode, 0.3);
		DEO.play(deoVideoNode);
	}
} 
*/
function onClickExternalStreamingCheckbox(checkbox)
{
	toggleExternalStreaming(checkbox.checked, true);
}
function toggleExternalStreaming(value, userClicked)
{
	if(userClicked) {
		externalStreaming = value;
		window.localStorage.setItem("externalStreaming", JSON.stringify(value));
	}
	else
		document.getElementById("externalStreamingCheckbox").checked = value;
	if(value) {
		videoNode.pause();
		videoNode.classList.remove("video-shown");
		thumbsContainerNode.style.maxHeight = "";
		if(playingmediaItem) 
			clearPlayingMediaItem()
		if(resizeObserver)
			resizeObserver.unobserve(videoNode);
	} else {
		onResizeVideo();
		resizeObserver = new ResizeObserver(onResizeVideo);
		resizeObserver.observe(videoNode);
	}
}

function playVideo(obj) {
	if(!externalStreaming) {
		if(playingmediaItem) {
			if(playingmediaItem.id === obj.id)
				return;
			clearPlayingMediaItem();
		}
		setPlayingMediaItem(obj);
		videoNode.classList.add("video-shown");
		videoSourceNode.setAttribute("src", "/media" + obj.relativePath);
		videoNode.setAttribute("title", obj.name);
		videoNode.setAttribute("poster", "/thumb/" + obj.relativeThumb);
		videoNode.load();
		// loadedFunscripts = [];
		// if(playingmediaItem.hasScript)
		// 	loadMediaFunscript(playingmediaItem.scriptNoExtensionRelativePath, playingmediaItem.isMFS);
		// else
			//videoNode.play();

	} else { 
		window.open("/video"+ obj.relativePath)
	}
}

function setPlayingMediaItem(obj) {
	playingmediaItem = obj;
	playingmediaItemNode = document.getElementById(obj.id);
	playingmediaItemNode.classList.add("media-item-playing");
}
function clearPlayingMediaItem() {
	playingmediaItemNode.classList.remove("media-item-playing");
	playingmediaItem = null;
	playingmediaItemNode = null;
}

var timer1 = 0;
var timer2 = Date.now();
function onVideoTimeUpdate(event) {
	if (timer2 - timer1 >= 1000) {
		timer1 = timer2;
		sendMediaState();
	}
	timer2 = Date.now();
}
function onVideoLoad(event) {
	console.log("Data loaded")
	console.log("Duration: "+ videoNode.duration )
	playingmediaItem.loaded = true;
}
function onVideoPlay(event) {
	console.log("Video play")
	playingmediaItem.playing = true;
	// if(!funscriptSyncWorker && loadedFunscripts && loadedFunscripts.length > 0)
	// 	startFunscriptSync(loadedFunscripts);
	sendMediaState();
}
function onVideoPause(event) {
	console.log("Video pause")
	playingmediaItem.playing = false;
	//setTimeout(function() {
		sendMediaState();// Sometimes a timeupdate is sent after this event fires?
	//}, 500);
}
function onVideoStall(event) {
	console.log("Video stall")
	playingmediaItem.playing = false;
	sendMediaState();
}
function onVideoPlaying(event) {
	console.log("Video playing")
	playingmediaItem.playing = true;
	sendMediaState();
}
function onVolumeChange() {
	window.localStorage.setItem("volume", videoNode.volume);
}
function onVideoEnd(event) {
	// if(funscriptSyncWorker) {
	// 	funscriptSyncWorker.postMessage(JSON.stringify({"command": "terminate"}));
	// }
	var playingIndex = mediaListObj.findIndex(x => x.path === playingmediaItem.path);
	playingIndex++;
	if(playingIndex < mediaListObj.length)
		playVideo(mediaListObj[playingIndex]);
	else
		playVideo(mediaListObj[0]);
}
function setThumbSize(value, userClick) {
	if(!userClick) {
		if(value)
			document.getElementById("thumbSize").value = value.toString();
	} else {
		window.localStorage.setItem("thumbSize", parseInt(value, 10));
		thumbSizeGlobal = parseInt(value, 10);
	}
}

// function startFunscriptSync() {
// 	if (window.Worker) {
// 		if(funscriptSyncWorker)
// 			funscriptSyncWorker.terminate();
// 		funscriptSyncWorker = new Worker('syncFunscript.js');
// 		funscriptSyncWorker.postMessage(JSON.stringify({
// 			"command": "startThread", 
// 			"funscripts": loadedFunscripts, 
// 			"remoteUserSettings": remoteUserSettings
// 		}));
// 		funscriptSyncWorker.onmessage = onFunscriptWorkerThreadRecieveMessage;
// 	}
// }

function sendMediaState() {
	//console.log("sendMediaState")
	if(playingmediaItem)
		postMediaState({
			"path": playingmediaItem.path,
			"playing": playingmediaItem.playing, 
			"currentTime": videoNode.currentTime, 
			"duration": videoNode.duration,
			"playbackSpeed": videoNode.speed
		});
}

function onFunscriptWorkerThreadRecieveMessage(e) {
    isMediaFunscriptPlaying = true;
    var data;
    if(typeof e.data === "string")
        data = JSON.parse(e.data);
    else
        data = e.data;
    switch(data["command"]) {
        case "sendTcode":
            sendTcode(data["tcode"])
            break;
		case "getMediaState":
			sendMediaState();
			break;
        case "end":
			funscriptSyncWorker.terminate();
			funscriptSyncWorker = null;
            break;
    }
}
//Settings
function openSettings() {
  settingsNode.style.visibility = "visible";
  settingsNode.style.opacity = 1;
  document.getElementById("settingsTabs").style.display = "block";
}

function closeSettings() {
  settingsNode.style.visibility = "hidden";
  settingsNode.style.opacity = 0;
  document.getElementById("settingsTabs").style.display = "none";
}

function tabClick(tab, tabNumber) {
	var allTabs = document.getElementsByClassName("tab-section-tab")
	for(var i=0;i<allTabs.length;i++) {
		if(i==tabNumber)
			continue;
		var otherTab = allTabs[i];
		otherTab.style.backgroundColor = "#5E6B7F";
	}
	var allContent = document.getElementsByClassName("tab-content")
	for(var i=0;i<allContent.length;i++) {
		if(i==tabNumber)
			continue;
		var content = allContent[i];
		content.style.display = 'none';
		content.style.opacity = "0";
	}
	allContent[tabNumber].style.display = 'block';
	allContent[tabNumber].style.opacity = "1";
	tab.style.backgroundColor = '#8DA1BF';
}

function showChange(selectNode) {
	sort(sortByGlobal, true);
	loadMedia(show(selectNode.value, true));
}

function sortChange(selectNode) {
	sort(selectNode.value, true);
	loadMedia(show(showGlobal, true));
}

function thumbSizeChange(selectNode) {
	setThumbSize(selectNode.value, true);
	loadMedia(show(showGlobal, true));
}

var sendTcodeDebouncer;
function setupSliders() {
	// Initialize Sliders
	var availableAxis = remoteUserSettings.availableAxisArray;
	var tcodeTab = document.getElementById("tabTCode");
	for(var i=0; i<availableAxis.length; i++) {
		var channel = availableAxis[i];

		var formElementNode =  document.createElement("div"); 
		formElementNode.classList.add("formElement");

		var labelNode = document.createElement("label");
		labelNode.classList.add("range-label")
		labelNode.innerText = channel.friendlyName;
		labelNode.for = channel.channel;
		formElementNode.appendChild(labelNode);

		var sectionNode = document.createElement("section");
		sectionNode.classList.add("range-slider");
		sectionNode.id = channel.channel;
		formElementNode.appendChild(sectionNode);

		var rangeValuesNode = document.createElement("span");
		rangeValuesNode.classList.add("range-values")
		sectionNode.appendChild(rangeValuesNode);

		var input1Node = document.createElement("input");
		input1Node.type = "range";
		input1Node.min = channel.min;
		input1Node.max = channel.max - 1;
		input1Node.value = channel.userMin;

		var input2Node = document.createElement("input");
		input2Node.type = "range";
		input2Node.min = channel.min + 1;
		input2Node.max = channel.max;
		input2Node.value = channel.userMax;

		if(userAgentIsDeo) {
			formElementNode.style.height ="50px"
			labelNode.style.height ="50px"
		} else {
			input1Node.classList.add("range-input");
			input2Node.classList.add("range-input");
		}
		input1Node.oninput = function(input1Node, input2Node, rangeValuesNode, channel) {
			var slide1 = parseInt( input1Node.value );
			var slide2 = parseInt( input2Node.value );
			var slideMid =  Math.round((slide2 + slide1) / 2);
			rangeValuesNode.innerText = slide1 + " - " + slideMid + " - " + slide2;
			if(slide2 < slide1) {
				input2Node.value = slide1 + 1;
				remoteUserSettings.availableAxis[channel.channel].userMax = input2Node.value;
			}
			remoteUserSettings.availableAxis[channel.channel].userMin = slide1;
			remoteUserSettings.availableAxis[channel.channel].userMid = slideMid;
			// if(sendTcodeDebouncer)
			// 	clearTimeout(sendTcodeDebouncer);
			// sendTcodeDebouncer = setTimeout(function () {
					sendTCode(channel.channel + input1Node.value.toString().padStart(4, '0') + "S2000")
			// }, 1000);
		}.bind(input1Node, input1Node, input2Node, rangeValuesNode, channel);
		
		input2Node.oninput = function(input1Node, input2Node, rangeValuesNode, channel) {
			var slide1 = parseInt( input1Node.value );
			var slide2 = parseInt( input2Node.value );
			var slideMid =  Math.round((slide2 + slide1) / 2);
			rangeValuesNode.innerText = slide1 + " - " + slideMid + " - " + slide2;
			if(slide1 > slide2) {
				input1Node.value = slide2 - 1;
				remoteUserSettings.availableAxis[channel.channel].userMin = input1Node.value;
			}
			remoteUserSettings.availableAxis[channel.channel].userMax = slide2;
			remoteUserSettings.availableAxis[channel.channel].userMid = slideMid;
			// if(sendTcodeDebouncer)
			// 	clearTimeout(sendTcodeDebouncer);
			// sendTcodeDebouncer = setTimeout(function () {
					sendTCode(channel.channel + input2Node.value.toString().padStart(4, '0') + "S2000")
			// }, 1000);
		}.bind(input2Node, input1Node, input2Node, rangeValuesNode, channel);

		sectionNode.appendChild(input1Node);
		sectionNode.appendChild(input2Node);

		var slide1 = parseInt( input1Node.value );
		var slide2 = parseInt( input2Node.value );
		var slideMid =  Math.round((slide2 + slide1) / 2);
		rangeValuesNode.innerText = slide1 + " - " + slideMid + " - " + slide2;

		tcodeTab.appendChild(formElementNode);
	}
}

function setupConnectionsTab() {
/*   
	connectionSettingsJson["networkAddress"] = SettingsHandler::getServerAddress();
	connectionSettingsJson["networkPort"] = SettingsHandler::getServerPort();
	connectionSettingsJson["serialPort"] = SettingsHandler::getSerialPort(); */
	switch(remoteUserSettings.connection.selectedSyncDevice) {
		case DeviceType.Deo:
			document.getElementById("connectionDeoVR").checked = true;
			break;
		case DeviceType.Whirligig:
			document.getElementById("connectionWhirligig").checked = true;
			break;
		case DeviceType.XTPWeb:
			document.getElementById("connectionXTPWeb").checked = true;
			break;
		case DeviceType.None:
			document.getElementById("connectionNone").checked = true;
			break;
	}
	document.getElementById("deoVRAddress").value = remoteUserSettings.connection.deoAddress
	document.getElementById("deoVRPort").value = remoteUserSettings.connection.deoPort
	document.getElementById("connectionGamepad").checked = remoteUserSettings.connection.gamePadEnabled;
}

var debouncer;
function webSocketAddressChange(e) {
	if(debouncer) {
		clearTimeout(debouncer);
	}
	debouncer = setTimeout(function () {
		debouncer = null;
		deviceAddress = document.getElementById("webSocketAddress").value;
		window.localStorage.setItem("webSocketAddress", value);
	}, 500);
}

function defaultLocalSettings() {
	var r = confirm("Are you sure you want to reset ALL local settings to default? Note: This only resets the settings in this browser. Your settings stored in XTP will remain.");
	if (r) {
		window.localStorage.clear();
		window.location.reload();
	} 
}

function deleteLocalSettings() {
	var r = confirm("Are you sure you want to delete ALL settings from localStorage and close the window?");
	if (r) {
		window.localStorage.clear();
		window.close();//Does not work
	} 
}
function onDeoVRAddressChange(input) {
	remoteUserSettings.connection.deoAddress = input.value;
}
function onDeoVRPortChange(input) {
	remoteUserSettings.connection.deoPort = input.value;
}
// function connectToTcodeDevice() {
// 	webSocket = new WebSocket("ws://"+deviceAddress+"/ws");
// 	webSocket.onopen = function (event) {
// 		webSocket.send("D1");
// 	};
// 	webSocket.onmessage = function (event) {
// 		console.log(event.data);
// 		document.getElementById("webSocketStatus").innerHTML = "Connected";
// 	}
// 	webSocket.onerror = function (event) {
// 		console.log(event.data);
// 		webSocket = null;
// 		document.getElementById("webSocketStatus").innerHTML = "Error";
// 	}
// }

// function sendTcode(tcode) {
// 	console.log("Send tcode: "+tcode);
// 	if(webSocket)
// 	{
// 		webSocket.send(tcode);
// 	}
// }
