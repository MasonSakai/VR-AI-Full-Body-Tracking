
#include "OverlayManager.h"

const char* OverlayKey = "AIVRFBTOverlay";
const char* DashboardKey = "AIVRFBTDashboard";
const char* OverlayName = "AI Full Body Tracking";

std::string CameraKey = "AIVRFBTCam";
std::string CameraName = "AI FBT Camera ";

vr::VROverlayHandle_t overlayHandle;
vr::VROverlayHandle_t dashboardHandle, dashboardIconHandle;
vr::IVROverlay *VROverlay;

vr::VROverlayHandle_t cameraOverlays[16];

const float camWidth = .1f, camColorA = .25f;
struct CamDisplayTimeout {
	vr::VROverlayHandle_t handle;
	std::chrono::system_clock::time_point startTime;
	float timeout = 15;
};
std::queue<CamDisplayTimeout> CameraDisplayTimeoutQueue;
std::thread CameraDisplayTimeoutThread;
bool cameraThreadActive = false;


bool GetOverlays() {
	VROverlay = vr::VROverlay();
	//vr::VROverlayError_None
	vr::EVROverlayError error = VROverlay->FindOverlay(OverlayKey, &overlayHandle);
	if (error == vr::VROverlayError_UnknownOverlay) {
		error = VROverlay->CreateOverlay(OverlayKey, OverlayName, &overlayHandle);
	}
	
	error = VROverlay->FindOverlay(DashboardKey, &overlayHandle);
	if (error == vr::VROverlayError_UnknownOverlay) {
		error = VROverlay->CreateDashboardOverlay(DashboardKey, OverlayName, &dashboardHandle, &dashboardIconHandle);
	}

	return true;
}

void DestroyOverlays() {
	vr::EVROverlayError error = VROverlay->DestroyOverlay(overlayHandle);
	error = VROverlay->DestroyOverlay(dashboardHandle);
	for (int i = 0; i < 16; i++) {
		error = VROverlay->DestroyOverlay(cameraOverlays[i]);
	}
}


void CameraTimeoutThread() {
	cameraThreadActive = true;

	while (cameraThreadActive) {
		if (CameraDisplayTimeoutQueue.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
		else {
			CamDisplayTimeout t = CameraDisplayTimeoutQueue.front();
			std::chrono::duration<float> delta = std::chrono::system_clock::now() - t.startTime;
			if (t.timeout < delta.count()) {
				VROverlay->HideOverlay(t.handle);
				CameraDisplayTimeoutQueue.pop();
			}
		}
	}
}
void OverlayOnClose() {
	if (cameraThreadActive) {
		cameraThreadActive = false;
		CameraDisplayTimeoutThread.join();
	}
	DestroyOverlays();
}

void CreateCameraOverlay(int index) {
	vr::EVROverlayError error = VROverlay->FindOverlay((CameraKey + std::to_string(index)).c_str(), &(cameraOverlays[index]));
	if (error == vr::VROverlayError_UnknownOverlay) {
		error = VROverlay->CreateOverlay((CameraKey + std::to_string(index)).c_str(), (CameraName + std::to_string(index)).c_str(), &(cameraOverlays[index]));
		if (error != 0) std::cout << error << "\n\n" << std::flush;
	}
	VROverlay->SetOverlayFromFile(cameraOverlays[index], "C:\\Users\\Mason Sakai\\OneDrive\\Pictures\\wtf_4.gif");
	VROverlay->SetOverlayAlpha(cameraOverlays[index], camColorA);
	VROverlay->SetOverlayWidthInMeters(cameraOverlays[index], camWidth);
	VROverlay->HideOverlay(cameraOverlays[index]);
}
void ShowCameraOverlay(int index) {
	vr::HmdMatrix34_t matrix = ConvertMatrix(cameras[index].transform, cameras[index].position);
	VROverlay->SetOverlayTransformAbsolute(cameraOverlays[index], vr::ETrackingUniverseOrigin::TrackingUniverseStanding, &matrix);
	VROverlay->ShowOverlay(cameraOverlays[index]);

	CamDisplayTimeout timeout;
	timeout.handle = cameraOverlays[index];
	timeout.startTime = std::chrono::system_clock::now();
	CameraDisplayTimeoutQueue.push(timeout);
	if (!cameraThreadActive)
		CameraDisplayTimeoutThread = std::thread(CameraTimeoutThread);
}