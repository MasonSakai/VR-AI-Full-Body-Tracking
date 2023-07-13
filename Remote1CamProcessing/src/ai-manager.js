import '@tensorflow/tfjs-backend-webgl';
import * as tf from '@tensorflow/tfjs-core';
import * as posedetection from '@tensorflow-models/pose-detection';

export class PoseDetector {

	constructor(flipHorizontal = false, width = 640) {
		tf.setBackend("webgl");
		this.flipHorizontal = flipHorizontal;
		this.width = width;
	}

	async createDetector() {
		await tf.ready();
		let modelType = posedetection.movenet.modelType.SINGLEPOSE_THUNDER;
		this.detector = await posedetection.createDetector(posedetection.SupportedModels.MoveNet, { modelType });
	}

	async estimatePose(video) {
		return await this.detector.estimatePoses(video);
	}

	async getFilteredPose(video, threshold) {
		let raw = (await this.estimatePose(video));
		if (raw.length == 0) return {};
		let keypoints = raw[0].keypoints;
		let filtered = keypoints.filter((data) => {
			return data.score >= threshold;
		});
		let pose = {};
		filtered.forEach((data) => {
			pose[data.name] = {
				x: this.flipHorizontal ? this.width - data.x : data.x,
				y: data.y,
				score: data.score
			}
		})
		return pose;
	}

	tfReady() {
		return tf.ready();
	}
}