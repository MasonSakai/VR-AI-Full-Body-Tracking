#pragma once

#include <openvr.h>
#include <string>
#include <queue>
#include <thread>
#include <chrono>
#include "PoseTracker.h"
#include "mathUtil.h"
#include "VRFloatingOverlay.h"
#include "VRDashboardOverlay.h"


extern const char* OverlayKey;
extern const char* DashboardKey;
extern const char* OverlayName;

extern vr::IVROverlay* VROverlay;

extern vr::VROverlayHandle_t cameraOverlays[];

bool GetOverlays();
void DestroyOverlays();

void OverlayOnClose();

void CreateCameraOverlay(int index);
void ShowCameraOverlay(int index);
