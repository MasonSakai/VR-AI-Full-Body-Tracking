#pragma once

#include "OverlayManager.h"
#include "vrUtil.h"

extern bool calibrating;
extern std::thread* CalibrationThread;

void CalibrateCamera(uint8_t n);

uint8_t GetAvailableCameraIndex();
void RequestAddCamera();

void OnCameraStop();
void OnCameraDisconnect();

void OnRecenter();
void RecalibrateVirtualControllers();