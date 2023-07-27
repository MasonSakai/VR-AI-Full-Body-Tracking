#pragma once

#define BOOST_USE_WINDOWS_H

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <thread>
#include <queue>
#include <openvr.h>
#include <vrinputemulator.h>
#include <vector>
#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <csignal>

#include "PoseTracker.h"
#include "Overlay.h"
#include "util.h"


extern vr::IVRSystem* m_VRSystem;
extern vrinputemulator::VRInputEmulator inputEmulator;

extern const float _headsetHeight;
extern const float _handDownHeight;
extern const float _shoulderHeight;

extern glm::vec3 headPos, leftHandPos, rightHandPos;
extern glm::quat headRot, leftHandRot, rightHandRot;

extern bool trackersOverride;
extern bool active;
extern std::thread TrackerUpdateLoopThread;

extern bool IOTest;
extern bool NoBreak;

extern bool calibrating;
extern std::queue<uint8_t> calibrationQueue;
extern std::thread CalibrationThread;


enum ButtonMasks : uint64_t {
	OculusBY      = 0b00000000000000000000000000000000010,
	OculusAX      = 0b00000000000000000000000000010000000,
	OculusStick   = 0b00100000000000000000000000000000000,
	OculusTrigger = 0b01000000000000000000000000000000000,
	OculusBumper  = 0b10000000000000000000000000000000100 //figure out why this has two
};