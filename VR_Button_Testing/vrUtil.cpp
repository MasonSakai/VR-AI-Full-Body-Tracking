#include "vrUtil.h"

vr::IVRSystem* m_VRSystem;

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
	return true;
}
void DisconnectFromVRRuntime()
{
	vr::VR_Shutdown();
}
