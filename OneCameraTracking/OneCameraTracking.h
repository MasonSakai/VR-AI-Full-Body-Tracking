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


static vr::IVRSystem* m_VRSystem;
static vrinputemulator::VRInputEmulator inputEmulator;

static uint32_t hipID;
static uint32_t leftFootID;
static uint32_t rightFootID;

const float _headsetHeight = 1.65f;
const float _handDownHeight = .71f;
const float _shoulderHeight = .9f;

float shoulderToShoulder,
shoulderToHip, hipWidth,
upperArmLen, lowerArmLen,
upperLegLen, lowerLegLen;

float degPerPixel;

glm::vec3 cameraPos;
glm::quat cameraRot;

glm::vec3 headPos, leftHandPos, rightHandPos;
glm::quat headRot, leftHandRot, rightHandRot;

glm::vec3 hipPos, leftFootPos, rightFootPos;
glm::quat hipRot, leftFootRot, rightFootRot;

glm::vec3 handOffset;

bool active = true;
std::thread TrackerUpdateLoopThread;

bool IOTest = false;
bool NoBreak = false;

bool calibrating = false;
std::queue<uint8_t> calibrationQueue;
std::thread CalibrationThread;


enum ButtonMasks : uint64_t {
	OculusBY      = 0b00000000000000000000000000000000010,
	OculusAX      = 0b00000000000000000000000000010000000,
	OculusStick   = 0b00100000000000000000000000000000000,
	OculusTrigger = 0b01000000000000000000000000000000000,
	OculusBumper  = 0b10000000000000000000000000000000100 //figure out why this has two
};