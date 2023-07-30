#include "VRDashboardOverlay.h"


#include <Windows.h>
#include <iostream>
#include <thread>

class TestListener : public IDashboardListener {
	vr::IVRSystem* GetVRSystem() {
		return NULL;
	}
	vr::IVROverlay* GetVROverlay() {
		return NULL;
	}

	void OnRecenter() {
		std::cout << "On Recenter\n";
	}
	void OnCalibrateTrackers() {
		std::cout << "On Calibrate Teackers\n";
	}

	void OnCalibrateCamera(uint8_t index) {
		std::cout << "On Calibrate Camera " << (int)index << "\n";
	}
	void RequestCameraScreenshot(uint8_t index) {

	}
	uint8_t GetCameraState(uint8_t index) {
		return 0;
	}

	void OnResetPM() {
		std::cout << "On Reset PM\n";
	}
	void OnSetPM(bool enable) {
		std::cout << "On Set PM: " << enable << "\n";
	}

	void QuitProgram() {
		std::cout << "On Quit\n";
	}
};


int main(int argc, char *argv[])
{
#ifdef _WIN32
	if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) { //this doesn't work...
		freopen("CONOUT$", "w", stdout);
		freopen("CONERR$", "w", stderr);
	}
#endif
	TestListener listener;
	VRDashboardOverlay::CreateSharedInstance(&listener, argc, argv);

	return 0;
}
