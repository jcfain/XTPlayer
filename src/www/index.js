mediaListObj = [];
sortByCache = "nameAsc"
window.onload = function httpGetAsync(callback)
{
	mediaListObj = JSON.parse(window.localStorage.getItem("mediaList"));
	sortByCache = JSON.parse(window.localStorage.getItem("sortBy"));
	if(!mediaListObj || mediaListObj.length == 0) {
		loadMediaFromServer();
	} else if(!mediaListObj) {
		mediaListObj = [];
	} else {
		sort(sortByCache, false);
		loadMedia(mediaListObj);
	}
}

function loadMediaFromServer() {
	window.localStorage.setItem("mediaList", JSON.stringify([]));
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
			name: mediaList[i]["name"],
			thumbSize: mediaList[i]["thumbSize"],
			relativePath: mediaList[i]["relativePath"],
			relativeThumb: mediaList[i]["relativeThumb"],
			modifiedDate: new Date(mediaList[i]["modifiedDate"])
		}
		mediaListObj.push(obj);
	}
	window.localStorage.setItem("mediaList", JSON.stringify(mediaListObj));
	sort(sortByCache, false);
	loadMedia(mediaListObj)
  }
}

function loadMedia(mediaList) {
	var medialistNode = document.getElementById("mediaList");
	removeAllChildNodes(medialistNode);
	if(mediaList.length == 0)
	{
		var divnode = document.createElement("div"); 
		divnode.id = "NoMedia";
		divnode.innerText = "No media found, it could be still loading";
		medialistNode.appendChild(divnode);
		return;
	}
	var noMediaElement = document.getElementById("NoMedia");
	if(noMediaElement)
		medialistNode.removeChild(noMediaElement)
	
	for(var i=0; i<mediaList.length;i++)
	{
		var obj = mediaList[i];
		var divnode = document.createElement("div"); 
		divnode.id = obj.name+"item"+i
		divnode.className += "media-item"
		divnode.style.width = obj.thumbSize + 60 + "px";
		divnode.style.height = obj.thumbSize + 25 + "px";
		divnode.title = obj.name;
		var anode = document.createElement("a"); 
		anode.className += "mediaLink"
		anode.href = "/video"+ obj.relativePath;
		var image = document.createElement("img"); 
		image.src = "/thumb/" + obj.relativeThumb;
		image.style.maxWidth = obj.thumbSize + 60 + "px";
		image.style.maxHeight = obj.thumbSize + "px";
		image.style.objectFit = "contain";
		image.id = obj.name+"thumb"+i
		var namenode = document.createElement("div");
		namenode.innerText = obj.name;
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

function sortChange(selectNode) {
	sort(selectNode.value, true);
	loadMedia(mediaListObj);
}

function sort(sortBy, userClick) {
	if(!sortBy)
		sortBy = "nameAsc";
	switch(sortBy) {
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
			  var nameA = a.name.toUpperCase(); // ignore upper and lowercase
			  var nameB = b.name.toUpperCase(); // ignore upper and lowercase
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
			  var nameA = a.name.toUpperCase(); // ignore upper and lowercase
			  var nameB = b.name.toUpperCase(); // ignore upper and lowercase
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
		document.getElementById("sortBy").value = sortBy;
	else
		window.localStorage.setItem("sortBy", JSON.stringify(sortBy));
}