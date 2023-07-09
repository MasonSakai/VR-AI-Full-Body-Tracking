let detector, camera, stats;
let startInferenceTime, numInferences = 0;
let inferenceTimeSum = 0, lastPanelUpdate = 0;
let rafId;
let renderer = null;
let useGpuRenderer = false;

async function getCamera(deviceID = null) {
    if ('mediaDevices' in navigator && 'getUserMedia' in navigator.mediaDevices) {
        let properties = {
            video: {
                width: {
                    ideal: 640
                },
                height: {
                    ideal: 480
                }
            }
        }
        if (deviceID) properties.video.deviceID = deviceID;
        return await navigator.mediaDevices.getUserMedia(properties);
    }
    return null;
}

async function getCameras() {
    const devices = await navigator.mediaDevices.enumerateDevices();
    const videoDevices = devices.filter(device => device.kind === 'videoinput');
    return videoDevices.map(videoDevice => {
        return {
            label: videoDevice.label,
            id: videoDevice.deviceId
        };
    });
}

//console.log(await getCameras());

async function renderPrediction() {
    await checkGuiUpdate();

    if (!STATE.isModelChanged) {
        await renderResult();
    }

    rafId = requestAnimationFrame(renderPrediction);
};
