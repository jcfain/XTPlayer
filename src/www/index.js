var mediaListObj = [];
var thumbsContainerNode;
var sortByGlobal = "nameAsc";
var showGlobal = "All";
var deoVideoNode;
var deoSourceNode;
var useDeoWeb = false;
document.addEventListener("DOMContentLoaded", function() {
  httpGetAsync();
});
function httpGetAsync(callback)
{
	thumbsContainerNode = document.getElementById("thumbsContainer");
/* 	deoVideoNode = document.getElementById("deoVideoPlayer");
	deoSourceNode = document.getElementById("deoVideoSource");
	
	deoVideoNode.addEventListener("end", onVideoStop); */
	
	sortByGlobal = JSON.parse(window.localStorage.getItem("sortBy"));
	showGlobal = JSON.parse(window.localStorage.getItem("show"));
	useDeoWeb = JSON.parse(window.localStorage.getItem("useDeoWeb"));
	
	if(useDeoWeb) {
		toggleUseDeo(useDeoWeb, false);
		new ResizeObserver(onResizeDeo).observe(deoVideoNode)
	}
	
	loadMediaFromServer();
}

function onResizeDeo() {
	if(useDeoWeb) {
		thumbsContainerNode.style.maxHeight = "calc(100vh - "+ (+deoVideoNode.offsetHeight + 120) + "px)";
	}
}

function loadMediaFromServer() {
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
			relativePath: mediaList[i]["relativePath"],
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
	if(useDeoWeb && mediaListObj.length > 0)
		loadVideo(mediaListObj[0]);
	sort(sortByGlobal, false);
	loadMedia(show(showGlobal, false))
  }
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
		deoVideoNode.style.display = "none"
		return;
	}
	var noMediaElement = document.getElementById("NoMedia");
	if(noMediaElement)
		medialistNode.removeChild(noMediaElement)
	
	var createClickHandler = function(obj) { 
		return function() { 
			loadVideo(obj); 
			playVideo(obj); 
		} 
	};
	
	for(var i=0; i<mediaList.length;i++)
	{
		var obj = mediaList[i];
		var divnode = document.createElement("div"); 
		divnode.id = obj.id+"item"+i
		divnode.className += "media-item"
		divnode.style.width = obj.thumbSize + 60 + "px";
		divnode.style.height = obj.thumbSize + 25 + "px";
		divnode.title = obj.name;
		var anode = document.createElement("a"); 
		anode.className += "mediaLink"
		if(obj.isMFS)
			anode.className += " mediaLinkMFS"
		if(!obj.hasScript)
			anode.className += " mediaLinkNoScript"
		anode.onclick = createClickHandler(obj);
		var image = document.createElement("img"); 
		image.src = "/thumb/" + obj.relativeThumb;
		image.style.maxWidth = obj.thumbSize + 60 + "px";
		image.style.maxHeight = obj.thumbSize + "px";
		image.style.objectFit = "contain";
		image.id = obj.id;
		var namenode = document.createElement("div");
		namenode.innerText = obj.displayName;
		namenode.className += "name"
		
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
function showChange(selectNode) {
	sort(sortByGlobal, true);
	loadMedia(show(selectNode.value, true));
}

function sortChange(selectNode) {
	sort(selectNode.value, true);
	loadMedia(show(showGlobal, true));
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
		deoSourceNode.setAttribute("src", "/video" + obj.relativePath);
		deoVideoNode.setAttribute("title", obj.name);
		deoVideoNode.setAttribute("cover-image", obj.relativeThumb);
		
		if(!DEO.isStarted(deoVideoNode))
			DEO.setVolume(deoVideoNode, 0.3);
		DEO.play(deoVideoNode);
	}
}

function playVideo(obj) {
	if(useDeoWeb) {
		DEO.start(deoVideoNode);
	} else {
		//window.open("/video"+ obj.relativePath, "_self")
		window.open("/video"+ obj.relativePath)
	}
}
function onVideoStop() {
	//deoVideoNode.style.display = "none"
}