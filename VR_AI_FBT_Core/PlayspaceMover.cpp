#include "PlayspaceMover.h"

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