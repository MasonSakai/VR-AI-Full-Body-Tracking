
const { io } = require("socket.io-client");

const { Camera } = require("./camera-manager");
const { PoseDetector } = require("./ai-manager");


const controlPanel = document.getElementsByClassName("control-panel")[0];

const lblState = document.getElementById("lbl-state");

const btnStart = document.getElementById("btn-start");
const btnStop = document.getElementById("btn-stop");
const cbxAutostart = document.getElementById("cbx-autostart");

const camSelect = document.getElementById("camera-select");
const btnCamRef = document.getElementById("btn-camref");
const video = document.getElementsByTagName("video")[0];
const canvas = document.getElementsByTagName("canvas")[0];

const btnApply = document.getElementById("btn-apply");
const btnCancel = document.getElementById("btn-cancel");
const btnReset = document.getElementById("btn-reset");
const lblPutState = document.getElementById("lbl-put-state");
var lblPutStateTimeout;

const width = video.clientWidth;
console.log(width);

lblState.innerHTML = "<i>Loading Camera...</i>";

const camera = new Camera();
const poseDetector = new PoseDetector(true, width);

lblState.innerHTML = "<i>Loading Socket...</i>";

const nodeSocket = io();

var hostSocket;// = io();

lblState.innerHTML = "<i>Loading...</i>";

var HostName = window.location.hostname;

const DefaultConfig = {
	"id": -1,
	"port": -1,
	"cameraName": "",
	"autostart": false,
	"confidenceThreshold": 0.3
};
let config = DefaultConfig;
let configUpdate = {};

var activeState = false;

async function fetchAsync(port) {
	let response = await fetch(port);
	let data = await response.json();
	return data;
}
async function fetchAsyncText(port) {
	let response = await fetch(port);
	let data = await response.text();
	return data;
}
async function putAsync(port, data) {
	return await fetch(port, {
		method: 'PUT',
		headers: {
			'Content-type': 'application/json'
		},
		body: JSON.stringify(data)
	});
}
async function putAsyncText(port, data) {
	return await fetch(port, {
		method: 'PUT',
		headers: {
			'Content-type': 'text/text'
		},
		body: data
	});
}

function hidePutState() {
	lblPutState.classList.add("d-none");
}
function setPutState(text, timeout) {
	lblPutState.innerHTML = text;
	if (lblPutStateTimeout) clearTimeout(lblPutStateTimeout);
	lblPutState.classList.remove("d-none");
	lblPutStateTimeout = setTimeout(hidePutState, timeout);
}

function applyConfigChage() {
	let madeChange = false;
	let hadError = false;
	if ("autostart" in configUpdate) {
		madeChange = true;
		config.autostart = configUpdate.autostart;
		cbxAutostart.checked = config.autostart;
	}
	if ("cameraName" in configUpdate) {
		madeChange = true;
		config.cameraName = configUpdate.cameraName;
		if (configUpdate.cameraName != "") {
			camSelect.value = configUpdate.cameraID;
			if (activeState) {
				camera.getCameraStream(camSelect.value).then((camera) => {
					video.srcObject = camera;
					resizeCanvas();
				});
			}
		} else {
			video.srcObject = undefined;
		}
	}
}

btnApply.onclick = () => {
	try {
		applyConfigChage();
		putAsync("config.json", config)
			.then((e) => {
				switch (e.status) {
					case 200:
						setPutState("Successfully Applied Settings", 1000);
						break;
					case 400:
					case 404:
					case 405:
					default:
						console.log(e);
						setPutState("Failed to Apply Settings", 5000);
						break;
				}
			})
	} catch (err) {
		console.error(err);
		setPutState("Failed to Apply Settings", 5000);
	}
};
btnReset.onclick = () => {
	try {
		configUpdate = DefaultConfig;
		if (applyConfigChage()) {
			putAsync("config.json", config)
				.then((e) => {
					switch (e.status) {
						case 200:
							setPutState("Successfully Reset Settings", 1000);
							break;
						case 400:
						case 404:
						case 405:
						default:
							console.log(e);
							setPutState("Failed to Reset Settings", 5000);
							break;
					}
				})
		}
	} catch (err) {
		console.error(err);
		setPutState("Failed to Reset Settings", 5000);
	}
};
btnCancel.onclick = () => {
	configUpdate = {};
	cbxAutostart.checked = config.autostart;
	camera.getCameraIDByName(config.cameraName).then((id) => { camSelect.value = id; });
};

btnStart.onclick = () => {
	startAILoop();
};
btnStop.onclick = () => {
	activeState = false;
	//Stop();
};

camera.updateCameraSelector(camSelect);

btnCamRef.onclick = () => {
	btnCamRef.disabled = true;
	let v = camSelect.value;
	camera.updateCameraSelector(camSelect).then(() => {
		camSelect.value = v;
		btnCamRef.disabled = false;
	});
};

camSelect.onchange = () => {
	camera.getCameraNameByID(camSelect.value).then((name) => {
		if (name === "") {
			camera.updateCameraSelector(camSelect);
			camSelect.value = "";
			console.log("Invalid Camera Selected");
		} else {
			configUpdate.cameraName = name;
			configUpdate.cameraID = camSelect.value;
		}
	})
};
cbxAutostart.onchange = () => {
	configUpdate.autostart = cbxAutostart.checked;
};

async function drawPose(pose) {
	let ctx = canvas.getContext("2d");
	ctx.clearRect(0, 0, canvas.width, canvas.height);
	Object.keys(pose).forEach((key) => {
		let spl = key.split("_");
		if (spl[0] == "right") ctx.fillStyle = "red";
		else if (spl[0] == "left") ctx.fillStyle = "green";
		else ctx.fillStyle = "blue";
		let point = pose[key];
		ctx.beginPath();
		ctx.arc(point.x, point.y, 5, 0, 2 * Math.PI);
		ctx.fill();
	});
}

nodeSocket.on("closing", () => {
	console.log("Node.js Closing");
	nodeSocket.disconnect();

})
nodeSocket.on("disconnect", () => {
	console.log("disconnected");
	controlPanel.classList.add("d-none");
});
nodeSocket.on("connect", () => {
	controlPanel.classList.remove("d-none");
});

function debounce(func, wait, immediate) {
	var timeout;
	return function () {
		var context = this, args = arguments;
		var later = function () {
			timeout = null;
			if (!immediate) func.apply(context, args);
		};
		var callNow = immediate && !timeout;
		clearTimeout(timeout);
		timeout = setTimeout(later, wait);
		if (callNow) func.apply(context, args);
	};
};

function resizeCanvas() {
	let rect = video.getBoundingClientRect();
	canvas.width = rect.width;
	canvas.height = rect.height;
	poseDetector.width = rect.width;
	if (poseDetector) poseDetector.width = rect.width;
}
resizeCanvas();

window.addEventListener("resize", debounce(resizeCanvas, 250, false));

function sleep(ms) {
	return new Promise(resolve => setTimeout(resolve, ms));
}

async function sendPose(pose) {
	if (hostSocket && hostSocket.connected) {
		hostSocket.emit("pose", pose);
	}
}

async function AILoop() {
	let pose = await poseDetector.getFilteredPose(video, config.confidenceThreshold);
	await sendPose(pose);
	drawPose(pose);
}


function InitHost() {
	hostSocket.on("requestSize", () => {
		console.log("Request Size");
		let rect = video.getBoundingClientRect();
		hostSocket.emit("requestSize", {
			width: rect.width,
			height: rect.height
		});
	});
	hostSocket.onAny((data) => {
		console.log(data);
	});
	hostSocket.on("disconnect", () => {
		console.log("Host Disconnect");
		hostSocket.disconnect();
	});
}

// Will return true if successfully connected, will return false if:
//     already connected, port is empty, or error occurs (see console for details)
async function tryConnectHost() {
	if (hostSocket && hostSocket.connected) return false;
	if (config.port == "") return false;
	try {
		hostSocket = io(config.port);
		let waiting = true;
		let timeout = setTimeout(() => { waiting = false; }, 10000);
		while (waiting) {
			if (hostSocket.connected) {
				clearTimeout(timeout);
				InitHost();
				return true;
			}
		}
		console.error("Couldn't connect");
		hostSocket.disconnect();
		hostSocket = undefined;
	} catch (err) {
		console.error(err);
		hostSocket = undefined;
	}
	return false;

}
async function tryReconnectHost() {
	if (hostSocket && hostSocket.connected) hostSocket.disconnect();
	return await tryConnectHost();
}

async function startAILoop() {
	try {
		activeState = true;

		if (!video.srcObject) {
			let camid = await camera.getCameraIDByName(config.cameraName);
			video.srcObject = await camera.getCameraStream(camid);
		}
		if (!poseDetector.detector) await poseDetector.createDetector();
		if (!(hostSocket && hostSocket.connected)) {
			//tryConnectHost();
			if (config.port > 0) {
				hostSocket = io(HostName + ":" + config.port);
				InitHost();
			}
		}

		resizeCanvas();
		canvas.classList.remove("d-none");
		let start, end, delta;

		/*poseDetector.estimatePose(video).then((data) => {
			console.log(data[0].keypoints.map((d) => {
				return d.name;
			}))});*/

		lblState.innerHTML = "Started Successfully"
		if (!(hostSocket && hostSocket.connected)) lblState.innerHTML += "<br>Without connection to host"

		while (activeState) {
			start = (performance || Date).now();
			await AILoop();
			end = (performance || Date).now();
			delta = end - start;
			if (delta < 16.66) {
				await sleep(16.66 - delta);
			}
		}
	} catch (err) {
		console.error(err);
		activeState = false;
		lblState.innerHTML = "Failed To Start..."
	}
}


async function GetConfig() {
	var response;
	try {
		response = await fetch("config.json");
	} catch (err) {
		console.error(err);
		nodeSocket.emit("initialized", "no-config-404");
		return;
	}

	//console.log(response);
	let data = await response.json();
	//console.log(data);
	let status = data.status;
	if (status != "ok") {
		console.log(`Error: ${status}`);
		lblState.innerHTML = `Loaded Error/Status:<br>${status}`;
	} else {
		lblState.innerHTML = "Loaded Config, reading...";
		config = data;
		let camid = await camera.getCameraIDByName(data.cameraName);
		camSelect.value = camid;
		//camSelect.dispatchEvent(new Event('change'));

		cbxAutostart.checked = data.autostart;

		if (data.autostart) {
			lblState.innerHTML = "Autostarting...";
			startAILoop();
		}
		lblState.innerHTML = "Loaded!";
	}
	setPutState("Connected To Server", 1000);

	switch (status) {
		case "ok":
			nodeSocket.emit("initialized", "successful");
			break;
		case "no-config":
			nodeSocket.emit("initialized", "no-config");
			break;
	}
}

lblState.innerHTML = "<i>Getting Config...</i>";
GetConfig();