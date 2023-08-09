#include "PlayspaceMover.h"

glm::vec3 pmStartControllerPos, pmOffset, pmOffsetStart;
uint8_t pmFlags = PlayspaceMoverFlags::Active | PlayspaceMoverFlags::DoubleButtonReset;

void InitPlayspaceMover() {
	QJsonObject pmConfig = config.object()["PlayspaceMover"].toObject();
	pmFlags = PlayspaceMoverFlags::None;
	if (pmConfig["enabled"].toBool()) pmFlags |= PlayspaceMoverFlags::Active;
	if (pmConfig["doubleButtonReset"].toBool()) pmFlags |= PlayspaceMoverFlags::DoubleButtonReset;
	pmButtonMask = GetButtonMaskFromConfig(pmConfig["buttons"].toObject());

	if (pmFlags & PlayspaceMoverFlags::Active) {
		EnableHardwareOffset();
	}
}
void ExitPlayspaceMover() {
	QJsonObject configObj = config.object();
	QJsonObject pmConfig = configObj["PlayspaceMover"].toObject();
	pmConfig.insert("enabled", (bool)(pmFlags & PlayspaceMoverFlags::Active));
	pmConfig.insert("doubleButtonReset", (bool)(pmFlags & PlayspaceMoverFlags::DoubleButtonReset));
	pmConfig.insert("buttons", GetConfigFromButtonMask(pmButtonMask));
	configObj.insert("PlayspaceMover", pmConfig);
	config.setObject(configObj);

	if (pmFlags & PlayspaceMoverFlags::Active) {
		pmOffset = glm::vec3();
		DisableHardwareOffset();
	}
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
				//std::cout << unDevice << std::endl << std::flush;
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

void CheckPlayspaceMover() {
	if (pmFlags & PlayspaceMoverFlags::Moving) {
		uint64_t buttons = GetControllerState((pmFlags & PlayspaceMoverFlags::ControllerRight) ? vr::TrackedControllerRole_RightHand : vr::TrackedControllerRole_LeftHand).ulButtonPressed;
		if (!(buttons & pmButtonMask)) {
			pmFlags &= ~PlayspaceMoverFlags::Moving;
			pmFlags &= ~PlayspaceMoverFlags::ControllerRight;
			buttonInputListener.pop();
		}
		else {
			if (pmFlags & PlayspaceMoverFlags::DoubleButtonReset) {
				buttons = GetControllerState((pmFlags & PlayspaceMoverFlags::ControllerRight) ? vr::TrackedControllerRole_LeftHand : vr::TrackedControllerRole_RightHand).ulButtonPressed;
				if (buttons & pmButtonMask) {
					pmOffset = glm::vec3(0, 0, 0);
					UpdateHardwareOffset();
					pmFlags &= ~PlayspaceMoverFlags::Moving;
					pmFlags &= ~PlayspaceMoverFlags::ControllerRight;
					pmFlags |= PlayspaceMoverFlags::DoubleButtonResetting;
					return;
				}
			}
			glm::vec3 controllerPos = (pmFlags & PlayspaceMoverFlags::ControllerRight) ? rightHandPosReal : leftHandPosReal;
			glm::vec3 delta = controllerPos - pmStartControllerPos;
			pmOffset = pmOffsetStart - delta;
			UpdateHardwareOffset();
		}
		return;
	}
	else if (pmFlags & PlayspaceMoverFlags::DoubleButtonResetting) {
		uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
		if (buttons & pmButtonMask) return;
		buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
		if (buttons & pmButtonMask) return;
		pmFlags &= ~PlayspaceMoverFlags::DoubleButtonResetting;
		buttonInputListener.pop();
	}
	else if (buttonInputListener.empty()) {
		uint64_t buttons = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
		if (buttons & pmButtonMask) {
			buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
			if (buttons & pmButtonMask) return;
			buttonInputListener.push(0);
			pmFlags |= PlayspaceMoverFlags::Moving;
			pmFlags &= ~PlayspaceMoverFlags::ControllerRight;
			pmStartControllerPos = leftHandPosReal;
			pmOffsetStart = pmOffset;
			return;
		}
		buttons = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
		if (buttons & pmButtonMask) {
			buttonInputListener.push(0);
			pmFlags |= PlayspaceMoverFlags::Moving;
			pmFlags |= PlayspaceMoverFlags::ControllerRight;
			pmStartControllerPos = rightHandPosReal;
			pmOffsetStart = pmOffset;
			return;
		}
	}
}