#include "OneCameraTracking.h"


enum PlayspaceMoverFlags: uint8_t{
	None = 0,
	Active = 128,
	Moving = 1,
	ControllerRight = 2
};

vr::IVRSystem* m_VRSystem;
vrinputemulator::VRInputEmulator inputEmulator;

uint32_t trackerIDs[17];

const float _headsetHeight = 1.65f;
const float _handDownHeight = .71f;
const float _shoulderHeight = .9f;

glm::vec3 headPos, leftHandPos, rightHandPos;
glm::vec3 headPosReal, leftHandPosReal, rightHandPosReal;
glm::quat headRot, leftHandRot, rightHandRot;
glm::quat headRotReal, leftHandRotReal, rightHandRotReal;

glm::vec3 controllerPosOffset;
glm::quat controllerRotOffset;

std::queue<uint8_t> buttonInputListener;

bool trackersOverride = false;
bool active = true;
std::thread TrackerUpdateLoopThread;

bool IOTest = false;
bool NoBreak = false;

bool calibrating = false;
std::queue<uint8_t> calibrationQueue;
std::thread *CalibrationThread;

//update
bool findTrackers() {
	if (inputEmulator.getVirtualDeviceCount() == 3) {
		for (int i = 0; i < 3; i++) {
			vr::DriverPose_t pose = inputEmulator.getVirtualDevicePose(i);
			std::cout << "Found Tracker " << inputEmulator.getVirtualDeviceInfo(i).deviceSerial << "\n" << std::flush;
			if (pose.deviceIsConnected == true || pose.result != vr::TrackingResult_Uninitialized || pose.poseIsValid == true) {
				std::cout << "Rejected: " << pose.deviceIsConnected << pose.result << pose.poseIsValid << std::endl << std::flush;
				return false;
			}
		}
		trackerIDs[Poses::right_hip] = 0;
		trackerIDs[Poses::left_ankle] = 1;
		trackerIDs[Poses::right_ankle] = 2;
		/*hipID = 0;
		leftFootID = 1;
		rightFootID = 2;*/
		std::cout << "Found Trackers\n" << std::flush;
		return true;
	}
	return false;
}

uint32_t createTracker() {
	uint32_t id = inputEmulator.getVirtualDeviceCount();
	inputEmulator.addVirtualDevice(vrinputemulator::VirtualDeviceType::TrackedController, std::to_string(id), false);
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
uint32_t createTracker(const char* deviceName) {
	uint32_t id = inputEmulator.getVirtualDeviceCount();
	inputEmulator.addVirtualDevice(vrinputemulator::VirtualDeviceType::TrackedController, deviceName, false);
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
	pos += controllerPosOffset + pmOffset; //see if input emulator effects these too
	rot *= controllerRotOffset;
	setVirtualDevicePosition(id, pos, rot);
}

void EnableHardwareOffset() {
	vr::HmdVector3d_t offset;
	offset.v[0] = 0;
	offset.v[1] = 0;
	offset.v[2] = 0;
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (!m_VRSystem->IsTrackedDeviceConnected(unDevice))
			continue;

		vr::VRControllerState_t state;
		if (m_VRSystem->GetControllerState(unDevice, &state, sizeof(state)))
		{
			vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);

			if (trackedDeviceClass < 3 && trackedDeviceClass > 0) {
				std::cout << unDevice << std::endl << std::flush;
				inputEmulator.enableDeviceOffsets(unDevice, true, false);
				inputEmulator.setWorldFromDriverTranslationOffset(unDevice, offset, false);
			}

		}
	}
}
void UpdateHardwareOffset() {

	vr::HmdVector3d_t offset;
	offset.v[0] = pmOffset.x;
	offset.v[1] = pmOffset.y;
	offset.v[2] = pmOffset.z;
	vrinputemulator::DeviceOffsets data;

	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (!m_VRSystem->IsTrackedDeviceConnected(unDevice))
			continue;

		vr::VRControllerState_t state;
		if (m_VRSystem->GetControllerState(unDevice, &state, sizeof(state)))
		{
			vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);

			if (trackedDeviceClass < 3 && trackedDeviceClass > 0) {
				inputEmulator.setWorldFromDriverTranslationOffset(unDevice, offset, false);

			}

		}
	}
}
void DisableHardwareOffset() {
	vr::HmdVector3d_t offset;
	offset.v[0] = 0;
	offset.v[1] = 0;
	offset.v[2] = 0;

	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (!m_VRSystem->IsTrackedDeviceConnected(unDevice))
			continue;

		vr::VRControllerState_t state;
		if (m_VRSystem->GetControllerState(unDevice, &state, sizeof(state)))
		{
			vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);

			if (trackedDeviceClass < 3 && trackedDeviceClass > 0) {
				inputEmulator.setWorldFromDriverTranslationOffset(unDevice, offset, false);
				inputEmulator.enableDeviceOffsets(unDevice, false, false);
			}

		}
	}
}

//Add
void onClose() {
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
}


/*void updateTrackers() {
	uint32_t HMDID = vr::k_unTrackedDeviceIndexInvalid;
	for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; deviceIndex++) {
		if (!vr::VRSystem()->IsTrackedDeviceConnected(deviceIndex)) {
			continue;
		}
		if (vr::VRSystem()->GetTrackedDeviceClass(deviceIndex) == vr::TrackedDeviceClass_HMD) {
			HMDID = deviceIndex;
			break;
		}
	}
	if (HMDID == vr::k_unTrackedDeviceIndexInvalid) {
		return;
	}

	vr::TrackedDevicePose_t* hmdPose = devicePoses + HMDID;
	glm::mat3x4* hmdMat = (glm::mat3x4*)&(hmdPose->mDeviceToAbsoluteTracking);
	glm::mat3x3 hmdRotMat = glm::mat3x3(*hmdMat);
	if (hmdPose->bPoseIsValid && hmdPose->bDeviceIsConnected) {
		glm::vec3 realHMDPos = devicePos[HMDID];
		glm::quat hmdRot = glm::inverse(glm::quat_cast(hmdRotMat));

		glm::vec3 down;
		glm::quat trackersRot;
		glm::vec3 footRight;
		glm::vec3 footForward;

		if (orbitTracker) {
			down = hmdRot * glm::vec3(0, -1, 0);
			trackersRot = hmdRot;
			footRight = trackersRot * glm::vec3(1, 0, 0);
			footForward = trackersRot * glm::vec3(0, 0, 1);
		}
		else {
			down = glm::vec3(0, -1, 0);
			glm::vec3 headRight = hmdRot * glm::vec3(1, 0, 0);
			headRight.y = 0;
			trackersRot = glm::quatLookAt(glm::normalize(headRight), glm::vec3(0, 1, 0));
			footRight = trackersRot * glm::vec3(0, 0, 1);
			footForward = trackersRot * glm::vec3(-1, 0, 0);
		}

		glm::vec3 hipPos = realHMDPos / 2.0f;
		glm::vec3 leftFootPos = (footForward - footRight) * 0.17f;
		glm::vec3 rightFootPos = (footForward + footRight) * 0.17f;
		setVirtualDevicePosition(hipID, hipPos, trackersRot);
		setVirtualDevicePosition(leftFootID, leftFootPos, trackersRot);
		setVirtualDevicePosition(rightFootID, rightFootPos, trackersRot);
	}
}

void printDevicePositionalData(const char* deviceName, vr::HmdMatrix34_t posMatrix, vr::HmdVector3_t position, vr::HmdQuaternion_t quaternion)
{

	// Print position and quaternion (rotation).
	printf("\n%s, x = %.5f, y = %.5f, z = %.5f, qw = %.5f, qx = %.5f, qy = %.5f, qz = %.5f",
		deviceName,
		position.v[0], position.v[1], position.v[2],
		quaternion.w, quaternion.x, quaternion.y, quaternion.z);


	// Uncomment this if you want to print entire transform matrix that contains both position and rotation matrix.
	//dprintf("\n%lld,%s,%.5f,%.5f,%.5f,x: %.5f,%.5f,%.5f,%.5f,y: %.5f,%.5f,%.5f,%.5f,z: %.5f,qw: %.5f,qx: %.5f,qy: %.5f,qz: %.5f",
	//    qpc.QuadPart, whichHand.c_str(),
	//    posMatrix.m[0][0], posMatrix.m[0][1], posMatrix.m[0][2], posMatrix.m[0][3],
	//    posMatrix.m[1][0], posMatrix.m[1][1], posMatrix.m[1][2], posMatrix.m[1][3],
	//    posMatrix.m[2][0], posMatrix.m[2][1], posMatrix.m[2][2], posMatrix.m[2][3],
	//    quaternion.w, quaternion.x, quaternion.y, quaternion.z);

}

void printPositionalData()
{

	// Process SteamVR device states
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
			vr::HmdVector3_t position;
			vr::HmdQuaternion_t quaternion;
			vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);

			switch (trackedDeviceClass) {
			case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
				vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0, &trackedDevicePose, 1);
				// print positiona data for the HMD.
				poseMatrix = trackedDevicePose.mDeviceToAbsoluteTracking; // This matrix contains all positional and rotational data.
				position = GetPosition(trackedDevicePose.mDeviceToAbsoluteTracking);
				quaternion = GetRotation(trackedDevicePose.mDeviceToAbsoluteTracking);

				printDevicePositionalData("HMD", poseMatrix, position, quaternion);

				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker:
				vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0, &trackedDevicePose, 1);
				// print positiona data for a general vive tracker.
				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
				vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, unDevice, &controllerState,
					sizeof(controllerState), &trackedControllerPose);
				poseMatrix = trackedControllerPose.mDeviceToAbsoluteTracking; // This matrix contains all positional and rotational data.
				position = GetPosition(trackedControllerPose.mDeviceToAbsoluteTracking);
				quaternion = GetRotation(trackedControllerPose.mDeviceToAbsoluteTracking);

				auto trackedControllerRole = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice);
				std::string whichHand = "";
				if (trackedControllerRole == vr::TrackedControllerRole_LeftHand)
				{
					whichHand = "LeftHand";
				}
				else if (trackedControllerRole == vr::TrackedControllerRole_RightHand)
				{
					whichHand = "RightHand";
				}

				switch (trackedControllerRole)
				{
				case vr::TrackedControllerRole_Invalid:
					// invalid
					break;

				case vr::TrackedControllerRole_LeftHand:
				case vr::TrackedControllerRole_RightHand:
					printDevicePositionalData(whichHand.c_str(), poseMatrix, position, quaternion);

					break;
				}

				break;
			}

		}
	}

}
*/

void UpdateRealHardwarePositions() {
	if (IOTest) return;
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
				inputEmulator.getDeviceOffsets(unDevice, data);
				poseMatrix = trackedDevicePose.mDeviceToAbsoluteTracking; // This matrix contains all positional and rotational data.
				headPosReal = GetPositionGLM(trackedDevicePose.mDeviceToAbsoluteTracking) - GetPositionGLM(data.worldFromDriverTranslationOffset);
				headRotReal = GetRotationGLM(trackedDevicePose.mDeviceToAbsoluteTracking) * glm::inverse(GetRotationGLM(data.worldFromDriverRotationOffset));
				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
				vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, unDevice, &controllerState,
					sizeof(controllerState), &trackedControllerPose);
				inputEmulator.getDeviceOffsets(unDevice, data);
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
	if (IOTest) return;
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
//Fix in NoVR
vr::VRControllerState_t GetControllerState(vr::ETrackedControllerRole controller) {
	vr::VRControllerState_t buttons;
	buttons.ulButtonPressed = 0;
	if (IOTest) return buttons;
	auto id = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(controller);
	if (id != vr::k_unTrackedDeviceIndexInvalid) {
		vr::VRSystem()->GetControllerState(id, &buttons, sizeof(vr::VRControllerState_t));
	}
	return buttons;
}

glm::vec3 pmStartControllerPos, pmOffset, pmOffsetStart;
uint8_t pmFlags = PlayspaceMoverFlags::Active;

void CheckPlayspaceMover() {
	if (pmFlags & PlayspaceMoverFlags::Moving) {
		uint64_t buttons = GetControllerState((pmFlags & PlayspaceMoverFlags::ControllerRight) ? vr::TrackedControllerRole_RightHand : vr::TrackedControllerRole_LeftHand).ulButtonPressed;
		if (!(buttons & ButtonMasks::OculusAX)) {
			pmFlags &= ~PlayspaceMoverFlags::Moving;
			pmFlags &= ~PlayspaceMoverFlags::ControllerRight;
			buttonInputListener.pop();
			std::cout << "Playspace Mover: Up\n" << std::flush;
		}
		else {
			glm::vec3 controllerPos = (pmFlags & PlayspaceMoverFlags::ControllerRight) ? rightHandPosReal : leftHandPosReal; //stop the gittering here
			glm::vec3 delta = controllerPos - pmStartControllerPos;
			pmOffset = pmOffsetStart - delta;
			UpdateHardwareOffset();
		}
		return;
	}
	else if (buttonInputListener.empty()) {
		uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
		if (buttons == ButtonMasks::OculusAX) {
			buttonInputListener.push(0);
			pmFlags |= PlayspaceMoverFlags::Moving;
			pmFlags &= ~PlayspaceMoverFlags::ControllerRight;
			pmStartControllerPos = leftHandPosReal;
			pmOffsetStart = pmOffset;
			std::cout << "Playspace Mover: Left Down\n" << std::flush;
			return;
		}
		buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
		if (buttons == ButtonMasks::OculusAX) {
			buttonInputListener.push(0);
			pmFlags |= PlayspaceMoverFlags::Moving;
			pmFlags |= PlayspaceMoverFlags::ControllerRight;
			pmStartControllerPos = rightHandPosReal;
			pmOffsetStart = pmOffset;
			std::cout << "Playspace Mover: Right Down\n" << std::flush;
			return;
		}
	}
}


void TrackerUpdateLoop() {
	uint8_t trackerStates[17];
	bool singleTrackerStates[17];
	uint8_t i, j;
	bool completed;

	auto lastTime = std::chrono::high_resolution_clock::now();
	int numFramePresents = 0;
	float deltaTime;
	uint32_t currentFrame = 0;

	while (active) {
		if (vr::VRCompositor() != NULL) {
			vr::Compositor_FrameTiming t;
			t.m_nSize = sizeof(vr::Compositor_FrameTiming);
			bool hasFrame = vr::VRCompositor()->GetFrameTiming(&t, 0);
			auto currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> dt = currentTime - lastTime;
			deltaTime = dt.count();
			// If the frame has changed we update, if a frame was redisplayed we update.
			if ((hasFrame && currentFrame != t.m_nFrameIndex) || (hasFrame && t.m_nNumFramePresents != numFramePresents)) {
				currentFrame = t.m_nFrameIndex;
				numFramePresents = t.m_nNumFramePresents;
				lastTime = currentTime;

				UpdateHardwarePositions();
				UpdateRealHardwarePositions();

				if (pmFlags & PlayspaceMoverFlags::Active) {
					CheckPlayspaceMover();
				}

				//Do Math
				for (i = 0; i < 17; i++) {
					singleTrackerStates[i] = false;
					trackerStates[i] = trackers[i].CalculatePosition();
				}

				for (j = 0; j < 10; j++) {
					completed = true;
					for (i = 0; i < 17; i++) {
						if (trackerStates[i] < 2 && !singleTrackerStates[i]) {
							if (trackerStates[i] == 1) {
								singleTrackerStates[i] = trackers[i].CalculateSingleCameraPosition();
								completed &= singleTrackerStates[i];
							}
							else {
								//todo
							}
						}
					}
					if (completed) break;
				}

				for (i = 0; i < 17; i++) {
					if (trackerStates[i] == 2 || singleTrackerStates[i])
						trackers[i].CalculateOrientation();
				}

				if (!trackersOverride)
					for (i = 0; i < 17; i++) {
						if (PoseTrackers[i]) {
							setOffsetVirtualDevicePosition(trackerIDs[i], trackers[i].position, trackers[i].rotation);
						}
					}

				// Sleep for just under 1/90th of a second, so that maybe the next frame will be available.
				std::this_thread::sleep_for(std::chrono::microseconds(10000));
			}
			else {
				// Still waiting on the next frame, wait less this time.
				std::this_thread::sleep_for(std::chrono::microseconds(1111));
			}
		}
		
	}
	onClose();
}

// add eof protection/checks?
int8_t ReceiveInt8_t() {
	int8_t v = 0;
	int8_t c = getchar();
	v = c << 4;
	c = getchar();
	v += (c & 0x0f);
	return v;
}
int16_t ReceiveInt16_t() {
	int c;
	int16_t v = 0;
	for (int i = 0; i < 2; i++) {
		c = ReceiveInt8_t();
		v = v << 8;
		v += c;
	}
	return v;
}
int32_t ReceiveInt32_t() {
	uint8_t c;
	int32_t v = 0;
	for (int i = 0; i < 4; i++) {
		c = ReceiveInt8_t();
		v = v << 8;
		v += c;
	}
	return v;
}
float ReceiveFloat() {
	float f;
	uint8_t b[4];
	int c;
	for (int i = 0; i < 4; i++) {
		b[i] = ReceiveInt8_t();;
	}
	memcpy(&f, &b, sizeof(f));
	return f;
}

void ReceivePose() {
	uint8_t socket = ReceiveInt8_t();

	//0b1xxxxxxx empty point
	uint8_t flags;
	for (int i = 0; i < 17; i++) {
		flags = ReceiveInt8_t();
		if (flags & 0b10000000) {
			trackers[i].ClearPose(socket);
		}
		else
		{
			float x = ReceiveFloat();
			float y = ReceiveFloat();
			float s = ReceiveFloat();
			trackers[i].UpdatePose(socket, x, y, s);
		}
	}


}

void RequestCameraSize(byte camera) {
	cameras[camera].waitingForSize = true;
	std::cout << (byte)18 << camera << std::flush;
}

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
		RequestCameraSize(camera);
		std::cout << "Begining Calibration of camera " << (int)camera << std::endl;

		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Put top of controller against the camera and hold X/A\n" << std::flush;
		while (!IOTest) {
			if (!active) return;
			uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
			if (buttons & ButtonMasks::OculusAX) {
				captureSide = false;
				break;
			}
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if (buttons & ButtonMasks::OculusAX) {
				captureSide = true;
				break;
			}
		}
		std::cout << "Release X/A\n" << std::flush;
		UpdateHardwarePositions();
		if (captureSide) {
			position = rightHandPos;
			qp = rightHandRot;
		}
		else {
			position = leftHandPos;
			qp = leftHandRot;
		}
		while (!IOTest) {
			if (!active) return;
			uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
			if ((buttons & ButtonMasks::OculusAX) == 0) break;
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if ((buttons & ButtonMasks::OculusAX) == 0) break;
		}


		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Go where you are entirely within frame\n";
		std::cout << "Put a controller against the ground the\n";
		std::cout << "same way you did for the camera and hold X/A\n" << std::flush;
		while (!IOTest) {

			if (!active) return;
			uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
			if (buttons & ButtonMasks::OculusAX) {
				captureSide = false;
				break;
			}
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if (buttons & ButtonMasks::OculusAX) {
				captureSide = true;
				break;
			}
		}
		std::cout << "Release X/A\n" << std::flush;
		UpdateHardwarePositions();
		if (captureSide) {
			p3 = trackers[10].GetPose(camera);
			v3 = rightHandPos;
			q3 = rightHandRot;
		}
		else {
			p3 = trackers[9].GetPose(camera);
			v3 = leftHandPos;
			q3 = leftHandRot;
		}
		while (!IOTest) {
			if (!active) return;
			uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
			if ((buttons & ButtonMasks::OculusAX) == 0) break;
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if ((buttons & ButtonMasks::OculusAX) == 0) break;
		}


		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Stay in the same place, T-Pose, and hold X/A\n";
		std::cout << "Preferably have your hands level and facing outwards,\n";
		std::cout << "This will be used to calibrate your hands from your wrists\n" << std::flush;
		while (!IOTest) {
			if (!active) return;
			uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
			if (buttons & ButtonMasks::OculusAX) break;
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if (buttons & ButtonMasks::OculusAX) break;
		}
		std::cout << "Release X/A\n" << std::flush;
		UpdateHardwarePositions();
		p1 = trackers[9].GetPose(camera);
		v1 = leftHandPos;
		q1 = leftHandRot;

		p2 = trackers[10].GetPose(camera);
		v2 = rightHandPos;
		q2 = rightHandRot;

		while (!IOTest) {
			if (!active) return;
			uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
			if ((buttons & ButtonMasks::OculusAX) == 0) break;
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if ((buttons & ButtonMasks::OculusAX) == 0) break;
		}

		std::cout << "Hold a moment...\n" << std::flush;

		while (active && cameras[camera].connected && cameras[camera].waitingForSize) { }
		cameras[camera].Calibrate(position, qp,
			v1, p1, q1,
			v2, p2, q2,
			v3, p3, q3);

		if (!IOTest) {
			ShowCameraOverlay(camera);
		}

		cameras[camera].CalibrateDistances(v1, q1, v2, q2, v3);
		//get joint dimentions
		//Everything is on a plane made by v1, v2, and v3; use that fact
		//project/reject starting position to get dist, dot normal and dir, use to get intersect position

		/* * ankle is slightly off ground in calibration, correct for it?
		*		Do in full body measurements
		*/

		std::cout << "Done Calibrating Camera " << (int)camera << std::endl << std::flush;
		calibrationQueue.pop();
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
void RequestCalibrateCamera() {
	uint8_t n = ReceiveInt8_t();
	CalibrateCamera(n);
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
	if (!IOTest) CreateCameraOverlay(n);
}

void OnCameraStart() {
	uint8_t socket = ReceiveInt8_t();
	std::cout << (int)socket << " Start" << std::endl;
	CalibrateCamera(socket);
	if(!IOTest) CreateCameraOverlay(socket);
}
void OnCameraStop() {
	uint8_t socket = ReceiveInt8_t();
	std::cout << (int)socket << " Stopped" << std::endl;

	cameras[socket].active = false;
	cameras[socket].connected = true;
	for (uint8_t i = 0; i < 17; i++) {
		trackers[i].ClearPose(socket);
	}
	if (!IOTest) VROverlay->HideOverlay(cameraOverlays[socket]);
}
void OnCameraDisconnect() {
	uint8_t socket = ReceiveInt8_t();
	std::cout << (int)socket << " Disconnected" << std::endl;

	cameras[socket].active = false;
	cameras[socket].connected = false;
	for (uint8_t i = 0; i < 17; i++) {
		trackers[i].ClearPose(socket);
	}
}
void OnCalibrationSizeReturn() {
	uint8_t socket = ReceiveInt8_t();
	cameras[socket].width = ReceiveInt16_t();
	cameras[socket].height = ReceiveInt16_t();
	cameras[socket].waitingForSize = false;
}

void OnRecenter() {
	//record position and orientation of one camera
	//recalibrate said camera
	//use delta position and orientation to adjust the rest of the cameras
	//can only be used to recenter
}
void RecalibrateVirtualControllers() {
	buttonInputListener.push(2);
	while (buttonInputListener.front() != 2) {}

	trackersOverride = true;

	vr::ETrackedControllerRole side;
	while (true) {
		UpdateHardwarePositions();
		setVirtualDevicePosition(trackerIDs[Poses::left_ankle], leftHandPos, leftHandRot);
		setVirtualDevicePosition(trackerIDs[Poses::right_ankle], rightHandPos, rightHandRot);
		uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
		if (buttons & ButtonMasks::OculusAX) {
			side = vr::TrackedControllerRole_LeftHand;
			break;
		}
		buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
		if (buttons & ButtonMasks::OculusAX) {
			side = vr::TrackedControllerRole_RightHand;
			break;
		}
	}

	glm::vec3 leftStartPos = leftHandPos,
			  rightStartPos = rightHandPos;

	while (!(GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed & ButtonMasks::OculusAX)) {}

	UpdateHardwarePositions();

	glm::vec3 startMid = (leftStartPos + rightStartPos) * .5f,
		endMid = (leftHandPos + rightHandPos) * .5f;

	controllerPosOffset = endMid - startMid;

	//Do I need orientation correction?

	buttonInputListener.pop();
}


void HandleArgs(int argc, char* argv[]) {
	if (argc < 2) return;
	for (int i = 1; i < argc; i++) {
		std::cout << argv[i];
		if (!strcmp(argv[i], "HideConsole")) {
			std::cout << std::endl << " - Hiding Console";
			ShowWindow(GetConsoleWindow(), SW_HIDE);
		}
		else if (!strcmp(argv[i], "NoVR")) {
			std::cout << std::endl << " - Running in No VR mode (IO Testing Only)";
			IOTest = true;
		}
		else if (!strcmp(argv[i], "NoBreak")) {
			std::cout << std::endl << " - Removing Close Protection";
			NoBreak = true;
		}
		std::cout << std::endl << std::flush;
	}
}

void endProgram() {
	std::cout << "Driver Stopping...\n" << std::flush;
	active = false;
	calibrating = false;

	if (!IOTest) {
		OverlayOnClose();
		TrackerUpdateLoopThread.join();
	}
	if(calibrating && CalibrationThread != nullptr)
		CalibrationThread->join();
	std::cout << "Driver Stopped\n" << std::flush;
}

static bool EndProgram(DWORD signal) {
	switch (signal)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		if (!NoBreak) {
			std::cout << "Closing..." << std::endl;
			endProgram();
			//exit(0);
			return true;
		}
		return true; //true? figure out what this means

	default:
		return false;
	}
}

int main(int argc, char* argv[])
{

	HandleArgs(argc, argv);
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)EndProgram, true);

	if (!IOTest) {
		vr::EVRInitError error = vr::VRInitError_Compositor_Failed;
		std::cout << "Looking for SteamVR..." << std::flush;
		while (error != vr::VRInitError_None) {
			m_VRSystem = vr::VR_Init(&error, vr::VRApplication_Overlay); //Change to vr::VRApplication_Background later
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
		GetOverlays();


		if (!findTrackers()) {
			for (int i = 0; i < 17; i++) {
				if (PoseTrackers[i]) {
					trackerIDs[i] = createTracker(PoseNames[i].c_str());
				}
			}
			//hipID = createTracker("Hip");
			//leftFootID = createTracker("Left Foot");
			//rightFootID = createTracker("Right Foot");
		}

		if (pmFlags & PlayspaceMoverFlags::Active)
			EnableHardwareOffset();
		TrackerUpdateLoopThread = std::thread(TrackerUpdateLoop);
	}

	for (uint8_t i = 0; i < 17; i++) {
		trackers[i].Init(i);
	}

	std::cout << (byte)0 << std::flush;

	// 0b11111111 close program
	// 0b01000000 pose data
	// 0b01000001 New Camera Request
	// 0b01000010 Camera Disconnected
	// 0b01000011 Begin Calibration
	// 0b01000100 Camera Size Return
	// 0b01000101 Camera Start
	// 0b01000110 Camera Stop
	// 0b01000111 Recenter
	uint8_t c;
	while (active)
	{
		c = ReceiveInt8_t();
		switch (c)
		{
		case 0b01000000:
			ReceivePose();
			break;
		case 0b01000001:
			RequestAddCamera();
			break;
		case 0b01000010:
			OnCameraDisconnect();
			break;
		case 0b01000011:
			RequestCalibrateCamera();
			break;
		case 0b01000100:
			OnCalibrationSizeReturn();
			break;
		case 0b01000101:
			OnCameraStart();
			break;
		case 0b01000110:
			OnCameraStop();
			break;
		case 0b01000111:
			OnRecenter();
			break;
		case 0b11111111: //255; close program
			endProgram();
			return 0;
		default:
			std::cout << "Defaulted on " << (int)c << std::endl << std::flush;
			break;
		}
	}

	
}
