#pragma once

#include "openvr.h"
#include <iostream>
#include <chrono>
#include <thread>

extern vr::IVRSystem* m_VRSystem;

//vr::TrackedControllerRole_LeftHand
vr::VRControllerState_t GetControllerState(vr::ETrackedControllerRole controller);

bool StartVR();
void DisconnectFromVRRuntime();