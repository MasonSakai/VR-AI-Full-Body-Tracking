#pragma once

#include <openvr.h>
#include <string>
#include <queue>
#include <thread>
#include <chrono>
#include "PoseTracker.h"
#include "util.h"
//#include <DashboardOverlay.h> //figure this out

extern vr::VROverlayHandle_t overlayHandle;
extern vr::VROverlayHandle_t dashboardHandle, dashboardIconHandle;
extern vr::IVROverlay* VROverlay;

extern vr::VROverlayHandle_t cameraOverlays[];

bool GetOverlays();
void DestroyOverlays();

void OverlayOnClose();

void CreateCameraOverlay(int index);
void ShowCameraOverlay(int index);
