
const { io } = require("socket.io-client");

const { Camera } = require("./camera-manager");
//const aiWorker = new Worker(new URL('./ai-worker.js', import.meta.url));

const video = document.getElementsByTagName("video")[0];
const canvas = document.getElementsByTagName("canvas")[0];
const camSelect = document.getElementById("camera-select");

const camera = new Camera(video, camSelect);

camera.initDocument();

camSelect.onchange = () => {
    if (camSelect.value == "") return;
    camera.getCameraStream(camSelect.value).then((camera) => {
        video.srcObject = camera;
    });
};

 //For Host, get from config
const hostname = '127.0.0.1';
const port = 2674;

const webHostSocket = io();

function disconnect() {
    webHostSocket.disconnect();
}

webHostSocket.on("message", (data) => {
    console.log("Got Message: " + data);
    webHostSocket.send(data);
});
webHostSocket.on("text message", (data) => {
    console.log("Got text message: " + data);
});

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
}
resizeCanvas();

window.addEventListener("resize", debounce(resizeCanvas, 125, false));


async function GetConfig() {
    var response;
    try {
        response = await fetch("config.json");
    } catch (err) {
        console.error(err);
        webHostSocket.emit("initialized", "no-config-404");
        return;
    }

    //console.log(response);
    let data = await response.json();
    console.log(data);
    let status = data.status;
    if (status != "ok") {
        console.log(`Error: ${status}`);
    } else {
        if (data.autostart) {
            let id = await camera.getCameraIDByName(data.cameraName);
            camSelect.value = id;
            camSelect.dispatchEvent(new Event('change'));


		}
	}

    switch (status) {
        case "ok":
            webHostSocket.emit("initialized", "successful");
            break;
        case "no-config":
            webHostSocket.emit("initialized", "no-config");
            break;
	}
}

GetConfig();