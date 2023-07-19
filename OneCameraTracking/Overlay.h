#pragma once

#include <openvr.h>
#include <string>
#include "PoseTracker.h"
#include "util.h"

extern vr::VROverlayHandle_t overlayHandle;
extern vr::VROverlayHandle_t dashboardHandle, dashboardIconHandle;
extern vr::IVROverlay* VROverlay;

extern vr::VROverlayHandle_t cameraOverlays[];

bool GetOverlays();
void DestroyOverlays();

void CreateCameraOverlay(int index);
void SetCameraOverlay(int index);
