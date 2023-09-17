#include "vrUtil.h"

vr::IVRSystem* m_VRSystem;
vrinputemulator::VRInputEmulator inputEmulator;

uint32_t trackerIDs[17];

glm::vec3 headPos, leftHandPos, rightHandPos;
glm::vec3 headPosReal, leftHandPosReal, rightHandPosReal;
glm::quat headRot, leftHandRot, rightHandRot;
glm::quat headRotReal, leftHandRotReal, rightHandRotReal;

glm::vec3 controllerPosOffset;
glm::quat controllerRotOffset;

bool trackersOverride = false;
bool active = true;

std::queue<uint8_t> buttonInputListener;

uint64_t inputButtonMask = ButtonMasks::OculusAX;
uint64_t pmButtonMask = ButtonMasks::OculusBY;

uint64_t GetButtonMaskFromConfig(QJsonObject config) {
	uint64_t flags = 0;
	if (config["maskRaw"].toInteger()) {
		int64_t signedFlags = config["maskRaw"].toInteger();
		flags = *(uint64_t*)&signedFlags;
	}
	if (config["oculusax"].toBool()) flags |= ButtonMasks::OculusAX;
	if (config["oculusby"].toBool()) flags |= ButtonMasks::OculusBY;
	if (config["oculustrigger"].toBool()) flags |= ButtonMasks::OculusTrigger;
	if (config["oculusbumper"].toBool()) flags |= ButtonMasks::OculusBumper;
	return flags;
}
QJsonObject GetConfigFromButtonMask(uint64_t mask) {
	QJsonObject config;
	config.insert("oculusax", (bool)(mask & ButtonMasks::OculusAX));
	config.insert("oculusby", (bool)(mask & ButtonMasks::OculusBY));
	config.insert("oculustrigger", (bool)(mask & ButtonMasks::OculusTrigger));
	config.insert("oculusbumper", (bool)(mask & ButtonMasks::OculusBumper));
	int64_t signedMask = *(int64_t*)&mask;
	config.insert("maskRaw", signedMask);
	return config;
}

//todo UPDATE
bool findTrackers() {
	//if (inputEmulator.getVirtualDeviceCount() == 3) {
	for (int i = 0; i < inputEmulator.getVirtualDeviceCount(); i++) {
		//vr::DriverPose_t pose = inputEmulator.getVirtualDevicePose(i);
		//std::cout << "Found Tracker " << inputEmulator.getVirtualDeviceInfo(i).deviceSerial << "\n" << std::flush;
		for (int j = 0; j < 17; j++) {
			if (inputEmulator.getVirtualDeviceInfo(i).deviceSerial == TrackerNames[j]) {
				trackerIDs[j] = i;
				//std::cout << "    Matched\n" << std::flush;
				break;
			}
		}
		/*if (pose.deviceIsConnected == true || pose.result != vr::TrackingResult_Uninitialized || pose.poseIsValid == true) {
			std::cout << "Rejected: " << pose.deviceIsConnected << pose.result << pose.poseIsValid << std::endl << std::flush;
			return false;
		}*/
	}
	//trackerIDs[Poses::right_hip] = 0;
	//trackerIDs[Poses::left_ankle] = 1;
	//trackerIDs[Poses::right_ankle] = 2;
	std::cout << "Found Trackers\n" << std::flush;
	return true;
	//}
	//return false;
}

uint32_t createTracker(std::string deviceName) {
	uint32_t id = inputEmulator.getVirtualDeviceCount();
	inputEmulator.addVirtualDevice(vrinputemulator::VirtualDeviceType::TrackedController, deviceName.c_str(), false);
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_TrackingSystemName_String, "lighthouse");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_ModelNumber_String, "Vive Controller MV");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_RenderModelName_String, "vr_controller_vive_1_5");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_WillDriftInYaw_Bool, false);
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_ManufacturerName_String, "HTC");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_TrackingFirmwareVersion_String, "1465809478 htcvrsoftware@firmware-win32 2016-06-13 FPGA 1.6/0/0 VRC 1465809477 Radio 1466630404");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_HardwareRevision_String, "product 129 rev 1.5.0 lot 2000/0/0 0");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_DeviceIsWireless_Bool, true);
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_HardwareRevision_Uint64, (uint64_t)2164327680);
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_FirmwareVersion_Uint64, (uint64_t)1465809478);
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_DeviceClass_Int32, 3);
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_SupportedButtons_Uint64, (uint64_t)12884901895);
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_Axis0Type_Int32, 1);
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_Axis1Type_Int32, 3);
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_Axis2Type_Int32, 0);
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_Axis3Type_Int32, 0);
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_Axis4Type_Int32, 0);
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_IconPathName_String, "icons");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceOff_String, "{htc}controller_status_off.png");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceSearching_String, "{htc}controller_status_searching.gif");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceSearchingAlert_String, "{htc}controller_status_alert.gif");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceReady_String, "{htc}controller_status_ready.png");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceNotReady_String, "{htc}controller_status_error.png");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceStandby_String, "{htc}controller_status_standby.png");
	inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceAlertLow_String, "{htc}controller_status_ready_low.png");
	inputEmulator.publishVirtualDevice(id);
	return id;
}
uint32_t createTracker() {
	uint32_t id = inputEmulator.getVirtualDeviceCount();
	return createTracker(std::to_string(id));
}

uint32_t getTracker(std::string deviceName) {
	for (int i = 0; i < inputEmulator.getVirtualDeviceCount(); i++) {
		if (inputEmulator.getVirtualDeviceInfo(i).deviceSerial == deviceName) {
			return i;
		}
	}
	return createTracker(deviceName.c_str());
}

void GetTrackers() {
	findTrackers();
	for (int i = 0; i < 17; i++) {
		if (PoseTrackers[i]) {
			try {
				trackerIDs[i] = createTracker(TrackerNames[i]);
			}
			catch (...) {

			}
		}
	}
}

void deleteVirtualDevice(int id) {
	vr::DriverPose_t pose = inputEmulator.getVirtualDevicePose(id);
	pose.deviceIsConnected = false;
	pose.result = vr::TrackingResult_Uninitialized;
	pose.poseIsValid = false;
	inputEmulator.setVirtualDevicePose(id, pose, false);
}

void setVirtualDevicePosition(uint32_t id, glm::vec3 pos, glm::quat rot) {
	vr::DriverPose_t pose = inputEmulator.getVirtualDevicePose(id);
	pose.poseIsValid = true;
	pose.deviceIsConnected = true;
	pose.result = vr::TrackingResult_Running_OK;
	pose.vecPosition[0] = pos.x;
	pose.vecPosition[1] = pos.y;
	pose.vecPosition[2] = pos.z;
	pose.qRotation.x = rot.x;
	pose.qRotation.y = rot.y;
	pose.qRotation.z = rot.z;
	pose.qRotation.w = rot.w;
	inputEmulator.setVirtualDevicePose(id, pose);
	//Why is this offset?/by quest coords not oculus coords
	//possible fix: have seprate position and rotation offsets
	//to fix, have (foot) controllers mimic hand controllers and freeze
	//person lines up their controllers with the virtual ones, and that difference is measured
}
void setOffsetVirtualDevicePosition(uint32_t id, glm::vec3 pos, glm::quat rot) {
	pos += controllerPosOffset + pmOffset;
	//rot *= controllerRotOffset;
	setVirtualDevicePosition(id, pos, rot);
}

//Do Not Use
/*void onClose() {
	for (int i = 0; i < 17; i++) {
		if (PoseTrackers[i]) {
			deleteVirtualDevice(trackerIDs[i]);
		}
	}
	//offset = glm::mat4x4(1);
	//velocity = glm::vec3(0);
	//move();
	if (pmFlags & PlayspaceMoverFlags::Active)
		DisableHardwareOffset();
	inputEmulator.disconnect();
}*/

void UpdateRealHardwarePositions() {
	if (!active) return;
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (!m_VRSystem->IsTrackedDeviceConnected(unDevice))
			continue;

		vr::VRControllerState_t state;
		if (m_VRSystem->GetControllerState(unDevice, &state, sizeof(state)))
		{
			vr::TrackedDevicePose_t trackedDevicePose;
			vr::TrackedDevicePose_t trackedControllerPose;
			vr::VRControllerState_t controllerState;
			vr::HmdMatrix34_t poseMatrix;
			glm::vec3 position;
			glm::quat quaternion;
			vrinputemulator::DeviceOffsets data;
			vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);

			switch (trackedDeviceClass) {
			case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
				vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0, &trackedDevicePose, 1);
				// print positiona data for the HMD.
				try {
					inputEmulator.getDeviceOffsets(unDevice, data);
				}
				catch (const std::exception& exc) {
					std::cerr << exc.what() << "\n";
				}
				poseMatrix = trackedDevicePose.mDeviceToAbsoluteTracking; // This matrix contains all positional and rotational data.
				headPosReal = GetPositionGLM(trackedDevicePose.mDeviceToAbsoluteTracking) - GetPositionGLM(data.worldFromDriverTranslationOffset);
				headRotReal = GetRotationGLM(trackedDevicePose.mDeviceToAbsoluteTracking) * glm::inverse(GetRotationGLM(data.worldFromDriverRotationOffset));
				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
				vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, unDevice, &controllerState,
					sizeof(controllerState), &trackedControllerPose);
				try {
					inputEmulator.getDeviceOffsets(unDevice, data);
				}
				catch (const std::exception& exc) {
					std::cerr << exc.what() << "\n";
				}
				poseMatrix = trackedControllerPose.mDeviceToAbsoluteTracking; // This matrix contains all positional and rotational data.
				position = GetPositionGLM(trackedControllerPose.mDeviceToAbsoluteTracking) - GetPositionGLM(data.worldFromDriverTranslationOffset);
				quaternion = GetRotationGLM(trackedControllerPose.mDeviceToAbsoluteTracking) * glm::inverse(GetRotationGLM(data.worldFromDriverRotationOffset));

				auto trackedControllerRole = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice);

				switch (trackedControllerRole)
				{
				case vr::TrackedControllerRole_Invalid:
					// invalid
					break;
				case vr::TrackedControllerRole_LeftHand:
					leftHandPosReal = position;
					leftHandRotReal = quaternion;
					break;
				case vr::TrackedControllerRole_RightHand:
					rightHandPosReal = position;
					rightHandRotReal = quaternion;
					break;
				}

				break;
			}

		}
	}
}
void UpdateHardwarePositions() {
	if (!active) return;
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (!m_VRSystem->IsTrackedDeviceConnected(unDevice))
			continue;

		vr::VRControllerState_t state;
		if (m_VRSystem->GetControllerState(unDevice, &state, sizeof(state)))
		{
			vr::TrackedDevicePose_t trackedDevicePose;
			vr::TrackedDevicePose_t trackedControllerPose;
			vr::VRControllerState_t controllerState;
			vr::HmdMatrix34_t poseMatrix;
			glm::vec3 position;
			glm::quat quaternion;
			vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);

			switch (trackedDeviceClass) {
			case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
				vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0, &trackedDevicePose, 1);
				// print positiona data for the HMD.
				poseMatrix = trackedDevicePose.mDeviceToAbsoluteTracking; // This matrix contains all positional and rotational data.
				headPos = GetPositionGLM(trackedDevicePose.mDeviceToAbsoluteTracking);
				headRot = GetRotationGLM(trackedDevicePose.mDeviceToAbsoluteTracking);
				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
				vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, unDevice, &controllerState,
					sizeof(controllerState), &trackedControllerPose);
				poseMatrix = trackedControllerPose.mDeviceToAbsoluteTracking; // This matrix contains all positional and rotational data.
				position = GetPositionGLM(trackedControllerPose.mDeviceToAbsoluteTracking);
				quaternion = GetRotationGLM(trackedControllerPose.mDeviceToAbsoluteTracking);

				auto trackedControllerRole = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice);

				switch (trackedControllerRole)
				{
				case vr::TrackedControllerRole_Invalid:
					// invalid
					break;
				case vr::TrackedControllerRole_LeftHand:
					leftHandPos = position;
					leftHandRot = quaternion;
					break;
				case vr::TrackedControllerRole_RightHand:
					rightHandPos = position;
					rightHandRot = quaternion;
					break;
				}

				break;
			}

		}
	}
}

//vr::TrackedControllerRole_LeftHand
vr::VRControllerState_t GetControllerState(vr::ETrackedControllerRole controller) {
	vr::VRControllerState_t buttons;
	buttons.ulButtonPressed = 0;
	auto id = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(controller);
	if (id != vr::k_unTrackedDeviceIndexInvalid) {
		vr::VRSystem()->GetControllerState(id, &buttons, sizeof(vr::VRControllerState_t));
	}
	return buttons;
}


bool StartVR()
{
	vr::EVRInitError error = vr::VRInitError_Compositor_Failed;
	std::cout << "Looking for SteamVR..." << std::flush;
	while (error != vr::VRInitError_None) {
		m_VRSystem = vr::VR_Init(&error, vr::VRApplication_Overlay);
		if (error != vr::VRInitError_None) {
			std::cout << "\nFailed due to reason " << VR_GetVRInitErrorAsSymbol(error) << "\n" << std::flush;
			std::cout << "Trying again in a few seconds...\n" << std::flush;
			std::this_thread::sleep_for(std::chrono::seconds(4));
		}
	}
	std::cout << "Success!\n" << std::flush;
	std::cout << "Looking for VR Input Emulator..." << std::flush;
	while (true) {
		try {
			inputEmulator.connect();
			break;
		}
		catch (vrinputemulator::vrinputemulator_connectionerror e) {
			std::cout << "\nFailed to connect to open vr input emulator, ensure you've installed it. If you have, try running this fix: https://drive.google.com/open?id=1Gn3IOm6GbkINplbEenu0zTr3DkB1E8Hc \n" << std::flush;
			std::this_thread::sleep_for(std::chrono::seconds(4));
			continue;
		}
	}
	std::cout << "Success!\n" << std::flush;
	return true;
}
void DisconnectFromVRRuntime()
{
	vr::VR_Shutdown();
}
