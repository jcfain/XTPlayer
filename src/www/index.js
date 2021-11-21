var mediaListObj = [];
var thumbsContainerNode;
var sortByGlobal = "nameAsc";
var showGlobal = "All";
var settingsNode;
var thumbSizeGlobal = 0;
var videoNode;
var videoSourceNode;
var useVideoElement;
var resizeObserver;
//var useDeoWeb;
//var deoVideoNode;
//var deoSourceNode;
document.addEventListener("DOMContentLoaded", function() {
  loadPage();
});
function loadPage()
{
	settingsNode = document.getElementById("settingsModal");
	thumbsContainerNode = document.getElementById("thumbsContainer");

	sortByGlobal = JSON.parse(window.localStorage.getItem("sortBy"));
	showGlobal = JSON.parse(window.localStorage.getItem("show"));
	thumbSizeGlobal = JSON.parse(window.localStorage.getItem("thumbSize"));
	var volume = JSON.parse(window.localStorage.getItem("volume"));
	useVideoElement = JSON.parse(window.localStorage.getItem("useVideoElement"));

	videoNode = document.getElementById("videoPlayer");
	videoSourceNode = document.getElementById("videoSource");
	videoNode.addEventListener("timeupdate", onVideoTimeUpdate); 
	videoNode.addEventListener("loadeddata", onVideoLoad); 
	videoNode.addEventListener("play", onVideoPlay); 
	videoNode.addEventListener("pause", onVideoPause); 
	videoNode.addEventListener("volumechange", onVolumeChange); 
	videoNode.volume = volume ? volume : 0.5;
	
	toggleUseVideo(useVideoElement, false);
	
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
	
	loadMediaFromServer();
}
function onResizeVideo() {
	thumbsContainerNode.style.maxHeight = "calc(100vh - "+ (+videoNode.offsetHeight + 120) + "px)";
} 
/* 
function onResizeDeo() {
	if(useDeoWeb) {
		thumbsContainerNode.style.maxHeight = "calc(100vh - "+ (+deoVideoNode.offsetHeight + 120) + "px)";
	}
} 
*/

async function loadMediaFromServer() {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', "/media", true);
	xhr.responseType = 'json';
	xhr.onload = function() {
	  var status = xhr.status;
	  if (status === 200) {
		onVideosLoad(null, xhr.response);
	  } else {
		onVideosLoad(status, xhr.response);
	  }
	};
	xhr.send();
}

function onVideosLoad(err, mediaList)
{
  if (err !== null) {
    alert('Whoops!: ' + err);
  } else {
	mediaListObj = [];
	for(var i=0; i<mediaList.length;i++)
	{
		var obj = {
			id: mediaList[i]["name"].replace(/^[^a-z]+|[^\w:.-]+/gi, "")+"thumb"+i,
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
			hasScript: mediaList[i]["hasScript"]
		}
		if(obj.isMFS)
			obj.displayName = "(MFS) " + obj.name;
		mediaListObj.push(obj);
	}
/* 	
	if(useDeoWeb && mediaListObj.length > 0)
		loadVideo(mediaListObj[0]); 
*/
	updateSettingsUI();
  }
}

function updateSettingsUI() {
	setThumbSize(thumbSizeGlobal, false);
	sort(sortByGlobal, false);
	loadMedia(show(showGlobal, false))
}

function loadMedia(mediaList) {
	var medialistNode = document.getElementById("mediaList");
	removeAllChildNodes(medialistNode);
	if(!mediaList || mediaList.length == 0)
	{
		var divnode = document.createElement("div"); 
		divnode.id = "NoMedia";
		divnode.innerText = "No media found, it may be loading.";
		medialistNode.appendChild(divnode);
		//deoVideoNode.style.display = "none"
		return;
	}
	var noMediaElement = document.getElementById("NoMedia");
	if(noMediaElement)
		medialistNode.removeChild(noMediaElement)
	
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
			fontSize = (textHeight * 0.4) + "px";
		}
		var divnode = document.createElement("div"); 
		divnode.id = obj.id+"item"+i
		divnode.className += "media-item"
		divnode.style.width = width;
		divnode.style.height = height;
		divnode.title = obj.name;
		var anode = document.createElement("a"); 
		anode.className += "mediaLink"
		if(obj.isMFS)
			anode.className += " mediaLinkMFS"
		if(!obj.hasScript)
			anode.className += " mediaLinkNoScript"
		anode.style.width = width;
		anode.style.height = height;
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
function onClickUseVideoCheckbox(checkbox)
{
	toggleUseVideo(checkbox.checked, true);
}
function toggleUseVideo(value, userClicked)
{
	if(userClicked) {
		useVideoElement = value;
		window.localStorage.setItem("useVideoElement", JSON.stringify(value));
	}
	else
		document.getElementById("useVideoCheckbox").checked = value;
	if(!value) {
		videoNode.pause();
		videoNode.style.display = "none";
		thumbsContainerNode.style.maxHeight = "";
		if(resizeObserver)
			resizeObserver.unobserve(videoNode);
	} else {
		onResizeVideo();
		resizeObserver = new ResizeObserver(onResizeVideo);
		resizeObserver.observe(videoNode);
	}
}

function playVideo(obj) {
	if(useVideoElement) {
		videoNode.style.display = "block";
		videoSourceNode.setAttribute("src", "/video" + obj.relativePath);
		videoNode.setAttribute("title", obj.name);
		videoNode.setAttribute("poster", "/thumb/" + obj.relativeThumb);
		//videoSourceNode.setAttribute("type", "video/mp4");
		videoNode.load();
		videoNode.play();
	} else { 
		window.open("/video"+ obj.relativePath)
	}
}
function onVideoTimeUpdate(event) {
	//console.log("Time update: "+ videoNode.currentTime )
}
function onVideoLoad(event) {
	console.log("Data loaded")
	console.log("Duration: "+ videoNode.duration )
}
function onVideoPlay(event) {
	console.log("Video play")
}
function onVideoPause(event) {
	console.log("Video pause")
}
function onVolumeChange() {
	window.localStorage.setItem("volume", videoNode.volume);
}
/* function onVideoStop() {
	deoVideoNode.style.display = "none"
} */
function setThumbSize(value, userClick) {
	if(!userClick) {
		if(value)
			document.getElementById("thumbSize").value = value.toString();
	} else {
		window.localStorage.setItem("thumbSize", parseInt(value, 10));
		thumbSizeGlobal = parseInt(value, 10);
	}
}

//Settings
function openSettings() {
  settingsNode.style.display = "flex";
}
function closeSettings() {
  settingsNode.style.display = "none";
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

function defaultSettings() {
	var r = confirm("Are you sure you want to reset ALL settings to default?");
	if (r) {
		window.localStorage.clear();
		window.location.reload();
	} 
}
function deleteSettings() {
	var r = confirm("Are you sure you want to delete ALL settings from localStorage and close the window?");
	if (r) {
		window.localStorage.clear();
		window.close();
	} 
}