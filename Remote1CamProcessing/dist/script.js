feather.replace();

const controls = document.querySelector('.controls');
const cameraOptions = document.querySelector('.video-options>select');
const video = document.querySelector('video');
const canvas = document.querySelector('canvas');
const screenshotImage = document.querySelector('img');
const buttons = [...controls.querySelectorAll('button')];
let streamStarted = false;

const [play, pause, screenshot] = buttons;

const getCameraSelection = async () => {
    const devices = await navigator.mediaDevices.enumerateDevices();
    const videoDevices = devices.filter(device => device.kind === 'videoinput');
    const options = videoDevices.map(videoDevice => {
        console.log(videoDevice);
        return `<option value="${videoDevice.deviceId}">${videoDevice.label}</option>`;
    });
    cameraOptions.innerHTML = options.join('');
};

play.onclick = () => {
    if (streamStarted) {
        video.play();
        play.classList.add('d-none');
        pause.classList.remove('d-none');
        return;
    }
    if ('mediaDevices' in navigator && navigator.mediaDevices.getUserMedia) {
        const updatedConstraints = {
            video: {
                deviceId: {
                    exact: cameraOptions.value
                }
			}
        };
        startStream(updatedConstraints);
    }
};

const startStream = async (constraints) => {
    const stream = await navigator.mediaDevices.getUserMedia(constraints);
    handleStream(stream);
};

const handleStream = (stream) => {

    console.log(stream.getVideoTracks()[0].getSettings());

    video.srcObject = stream;
    play.classList.add('d-none');
    pause.classList.remove('d-none');
    screenshot.classList.remove('d-none');
    streamStarted = true;
};

getCameraSelection();

cameraOptions.onchange = () => {
    const updatedConstraints = {
        video: {
            deviceId: {
                exact: cameraOptions.value
            }
        }
    };
    startStream(updatedConstraints);
};

const pauseStream = () => {
    video.pause();
    play.classList.remove('d-none');
    pause.classList.add('d-none');
};

const doScreenshot = () => {
    canvas.width = video.videoWidth;
    canvas.height = video.videoHeight;
    canvas.getContext('2d').drawImage(video, 0, 0);
    screenshotImage.src = canvas.toDataURL('image/webp');
    screenshotImage.classList.remove('d-none');
};

pause.onclick = pauseStream;
screenshot.onclick = doScreenshot;