
export class Camera {

    constructor(video, camSelect) {
        this.video = video;
        this.camSelect = camSelect;
	}

	 async getCameraStream(deviceID = null) {
        if ('mediaDevices' in navigator && 'getUserMedia' in navigator.mediaDevices) {
            let properties = {
                video: {} /*{
                    width: {
                        ideal: 640
                    },
                    height: {
                        ideal: 480
                    }
                }*/
            }
            if (deviceID) properties.video.deviceId = { exact: deviceID };
            return await navigator.mediaDevices.getUserMedia(properties);
        }
    }
    async getCameraIDByName(deviceName) {
        if ('mediaDevices' in navigator && 'getUserMedia' in navigator.mediaDevices) {
            let cameras = await this.getCameras();
            for (let i = 0; i < cameras.length; i++) {
                if (cameras[i].label == deviceName)
                    return cameras[i].id;
			}
        }
    }

    async getCameras() {
        const devices = await navigator.mediaDevices.enumerateDevices();
        const videoDevices = devices.filter(device => device.kind === 'videoinput');
        if (videoDevices[0].deviceId == '') {
            await this.getCameraStream();
            return await this.getCameras();
        }
        return videoDevices.map((videoDevice) => {
            return {
                label: videoDevice.label,
                id: videoDevice.deviceId
            };
        });
    }

    async renderPrediction() {
        await checkGuiUpdate();

        if (!STATE.isModelChanged) {
            await renderResult();
        }

        rafId = requestAnimationFrame(renderPrediction);
    };

    async initDocument() {
        this.getCameras().then((cameras) => {
            this.camSelect.innerHTML = `<option value="">Select camera</option>`;
            cameras.forEach((camera) => {
                this.camSelect.innerHTML += `\n<option value=${camera.id}>${camera.label}</option>`;
            });
        });

    }
}