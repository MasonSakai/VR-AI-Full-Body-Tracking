#include "PlayspaceMover.h"

glm::vec3 pmStartControllerPos, pmOffset, pmOffsetStart;
uint8_t pmFlags = PlayspaceMoverFlags::Active;

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