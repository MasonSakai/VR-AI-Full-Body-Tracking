#include "CameraManager.h"

bool calibrating = false;
std::queue<uint8_t> calibrationQueue;
std::thread* CalibrationThread;


void CalibrationThreadFunct() {
	uint8_t camera = 0;
	bool captureSide = false; //false = left
	glm::vec3 position, v1, v2, v3;
	glm::vec2 p1, p2, p3;
	glm::quat qp, q1, q2, q3;

	calibrating = true;
	buttonInputListener.push(1);
	while (buttonInputListener.front() != 1) {}
	while (!calibrationQueue.empty()) {
		camera = calibrationQueue.front();
		//VRDashboardOverlay::SharedInstance()->SetCameraState(camera, 2);
		std::cout << "Begining Calibration of camera " << (int)camera << std::endl;

		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Put top of controller against the camera and hold X/A\n" << std::flush;
		while (active) {
			uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
			if (buttons & inputButtonMask) {
				captureSide = false;
				break;
			}
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if (buttons & inputButtonMask) {
				captureSide = true;
				break;
			}
		}
		std::cout << "Release X/A\n" << std::flush;
		UpdateHardwarePositions();
		if (captureSide) {
			position = rightHandPosReal;
			qp = rightHandRotReal;
		}
		else {
			position = leftHandPosReal;
			qp = leftHandRotReal;
		}
		while (active) {
			uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
			if ((buttons & inputButtonMask) == 0) break;
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if ((buttons & inputButtonMask) == 0) break;
		}


		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Go where you are entirely within frame\n";
		std::cout << "Put a controller against the ground the\n";
		std::cout << "same way you did for the camera and hold X/A\n" << std::flush;
		while (active) {
			uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
			if (buttons & inputButtonMask) {
				captureSide = false;
				break;
			}
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if (buttons & inputButtonMask) {
				captureSide = true;
				break;
			}
		}
		std::cout << "Release X/A\n" << std::flush;
		UpdateHardwarePositions();
		if (captureSide) {
			p3 = trackers[Poses::right_wrist].GetPose(camera);
			v3 = rightHandPosReal;
			q3 = rightHandRotReal;
		}
		else {
			p3 = trackers[Poses::left_wrist].GetPose(camera);
			v3 = leftHandPosReal;
			q3 = leftHandRotReal;
		}
		while (active) {
			uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
			if ((buttons & inputButtonMask) == 0) break;
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if ((buttons & inputButtonMask) == 0) break;
		}


		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Stay in the same place, T-Pose, and hold X/A\n";
		std::cout << "Preferably have your hands level and facing outwards,\n";
		std::cout << "This will be used to calibrate your hands from your wrists\n" << std::flush;
		while (active) {
			uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
			if (buttons & inputButtonMask) break;
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if (buttons & inputButtonMask) break;
		}
		std::cout << "Release X/A\n" << std::flush;
		UpdateHardwarePositions();
		p1 = trackers[Poses::left_wrist].GetPose(camera);
		v1 = leftHandPosReal;
		q1 = leftHandRotReal;

		p2 = trackers[Poses::right_wrist].GetPose(camera);
		v2 = rightHandPosReal;
		q2 = rightHandRotReal;

		while (active) {
			uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
			if ((buttons & inputButtonMask) == 0) break;
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if ((buttons & inputButtonMask) == 0) break;
		}

		std::cout << "Hold a moment...\n" << std::flush;

		while (active && cameras[camera].connected && cameras[camera].waitingForSize) {}
		cameras[camera].Calibrate(position, qp,
			v1, p1, q1,
			v2, p2, q2,
			v3, p3, q3);

		ShowCameraOverlay(camera);

		cameras[camera].CalibrateDistances(v1, q1, v2, q2, v3);
		//get joint dimentions
		//Everything is on a plane made by v1, v2, and v3; use that fact
		//project/reject starting position to get dist, dot normal and dir, use to get intersect position

		/* * ankle is slightly off ground in calibration, correct for it?
		*		Do in full body measurements
		*/

		//VRDashboardOverlay::SharedInstance()->SetCameraState(camera, 1);
		std::cout << "Done Calibrating Camera " << (int)camera << std::endl << std::flush;
		calibrationQueue.pop();
		while (!calibrationQueue.empty() && calibrationQueue.front() == camera) calibrationQueue.pop();
	}
	buttonInputListener.pop();
	calibrating = false;
}

void CalibrateCamera(uint8_t n) {
	calibrationQueue.push(n);
	if (!calibrating) {
		CalibrationThread = new std::thread(CalibrationThreadFunct);
	}
}

uint8_t GetAvailableCameraIndex() {
	for (uint8_t i = 0; i < 16; i++)
		if (!cameras[i].connected)
			return i;
	return 255;
}
void RequestAddCamera() {
	uint8_t n = GetAvailableCameraIndex();
	std::cout << (byte)17 << (byte)n << std::flush;
	if (n == 255) return;
	cameras[n].active = false;
	cameras[n].connected = true;
	for (int i = 0; i < 17; i++) {
		trackers[i].InitCamera(n);
	}
	CreateCameraOverlay(n);
}

void OnCameraStop() {
	uint8_t socket = 0;// ReceiveInt8_t();
	std::cout << (int)socket << " Stopped" << std::endl;

	cameras[socket].active = false;
	cameras[socket].connected = true;
	for (uint8_t i = 0; i < 17; i++) {
		trackers[i].ClearPose(socket);
	}
	VROverlay->HideOverlay(cameraOverlays[socket]);
}
void OnCameraDisconnect() {
	uint8_t socket = 0;// ReceiveInt8_t();
	std::cout << (int)socket << " Disconnected" << std::endl;

	cameras[socket].active = false;
	cameras[socket].connected = false;
	for (uint8_t i = 0; i < 17; i++) {
		trackers[i].ClearPose(socket);
	}
}

void OnRecenter() {
	//record position and orientation of one camera
	//recalibrate said camera
	//use delta position and orientation to adjust the rest of the cameras
	//can only be used to recenter
}
void RecalibrateVirtualControllers() {
	std::cout << "Queuing Calib...\n";
	buttonInputListener.push(2);
	while (buttonInputListener.front() != 2) {}
	std::cout << "Starting Calib...\n";

	trackersOverride = true;

	vr::ETrackedControllerRole side;
	while (true) {
		UpdateHardwarePositions();
		setVirtualDevicePosition(trackerIDs[Poses::left_ankle], leftHandPos, leftHandRot);
		setVirtualDevicePosition(trackerIDs[Poses::right_ankle], rightHandPos, rightHandRot);
		uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
		if (buttons & inputButtonMask) {
			side = vr::TrackedControllerRole_LeftHand;
			break;
		}
		buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
		if (buttons & inputButtonMask) {
			side = vr::TrackedControllerRole_RightHand;
			break;
		}
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	glm::vec3 leftStartPos = leftHandPosReal,
		rightStartPos = rightHandPosReal;
	std::cout << "Waiting...\n";

	while ((GetControllerState(side).ulButtonPressed & inputButtonMask) != 0) {}

	UpdateHardwarePositions();

	glm::vec3 startMid = (leftStartPos + rightStartPos) * .5f,
		endMid = (leftHandPosReal + rightHandPosReal) * .5f;

	controllerPosOffset = startMid - endMid;

	//Do I need orientation correction?

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	trackersOverride = false;
	buttonInputListener.pop();
	std::cout << "Ending Calib...\n";
}
