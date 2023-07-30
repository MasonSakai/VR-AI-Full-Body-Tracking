#pragma once

#include "openvr.h"

class IDashboardListener
{
public:
	virtual vr::IVRSystem* GetVRSystem() = 0;
	virtual vr::IVROverlay* GetVROverlay() = 0;

	virtual void OnRecenter() = 0;
	virtual void OnCalibrateTrackers() = 0;

	virtual void OnCalibrateCamera(uint8_t index) = 0;
	virtual void RequestCameraScreenshot(uint8_t index) = 0;
	virtual uint8_t GetCameraState(uint8_t index) = 0;

	virtual void OnResetPM() = 0;
	virtual void OnSetPM(bool enable) = 0;

	virtual void QuitProgram() = 0;

};