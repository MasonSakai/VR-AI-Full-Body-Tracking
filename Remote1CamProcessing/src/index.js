
const { io } = require("socket.io-client");

const { Camera } = require("./camera-manager");
const { PoseDetector } = require("./ai-manager");

const canvas = document.getElementsByTagName("canvas")[0];

const lblState = document.getElementById("lbl-state");

const btnStart = document.getElementById("btn-start");
const btnStop = document.getElementById("btn-stop");
const cbxAutostart = document.getElementById("cbx-autostart");

const camSelect = document.getElementById("camera-select");
const btnCamRef = document.getElementById("btn-camref");
const video = document.getElementsByTagName("video")[0];

const txtIP = document.getElementById("txt-ip");
const ipSelect = document.getElementById("ip-select");

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

var hostSocket

lblState.innerHTML = "<i>Loading...</i>";

const DefaultConfig = {
	"id": -1,
	"url": "",
	"cameraName": "",
	"autostart": false,
	"confidenceThreshold": 0.3,
	"closeOnDisconnect": true
};
let config = DefaultConfig;
let configUpdate = {};

var activeState = true;

async function fetchAsync(url) {
	let response = await fetch(url);
	let data = await response.json();
	return data;
}
async function fetchAsyncText(url) {
	let response = await fetch(url);
	let data = await response.text();
	return data;
}
async function putAsync(url, data) {
	return await fetch(url, {
		method: 'PUT',
		headers: {
			'Content-type': 'application/json'
		},
		body: JSON.stringify(data)
	});
}
async function putAsyncText(url, data) {
	return await fetch(url, {
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
	if ("url" in configUpdate) {
		madeChange = true;
		config.url = configUpdate.url;
		txtIP.value = config.url;
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
	txtIP.value = config.url;
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
txtIP.onchange = () => {
	configUpdate.url = txtIP.value;
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

nodeSocket.on("disconnect", () => {
	console.log("disconnected");
});

window.onclose = () => {
	nodeSocket.emit("message", "Closing...");
}

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
}
resizeCanvas();

window.addEventListener("resize", debounce(resizeCanvas, 250, false));

function sleep(ms) {
	return new Promise(resolve => setTimeout(resolve, ms));
}

async function sendPose(pose) {
	
}

async function AILoop() {
	let pose = await poseDetector.getFilteredPose(video, config.confidenceThreshold);
	drawPose(pose);
	sendPose(pose);
}


async function startAILoop() {
	activeState = true;

	resizeCanvas();
	canvas.classList.remove("d-none");
	let start, end, delta;

	while (activeState) {
		start = startInferenceTime = (performance || Date).now();
		AILoop();
		end = startInferenceTime = (performance || Date).now();
		delta = end - start;
		if (delta < 16.66) {
			await sleep(16.66 - delta);
		}
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

		txtIP.value = data.url;
		cbxAutostart.checked = data.autostart;

		if (data.autostart) {
			lblState.innerHTML = "Autostarting...";
			video.srcObject = await camera.getCameraStream(camid);
			await poseDetector.createDetector()
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