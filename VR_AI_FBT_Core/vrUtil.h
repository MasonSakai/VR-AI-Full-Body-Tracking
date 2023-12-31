#pragma once

#define BOOST_USE_WINDOWS_H

#include "openvr.h"
#include "vrinputemulator.h"
#include "mathUtil.h"
#include "PlayspaceMover.h"
#include <queue>
#include "PoseTracker.h"

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum ButtonMasks : uint64_t {
	OculusBY      = 0b00000000000000000000000000000000010,
	OculusAX      = 0b00000000000000000000000000010000000,
	OculusStick   = 0b00100000000000000000000000000000000,
	OculusTrigger = 0b01000000000000000000000000000000000,
	OculusBumper  = 0b10000000000000000000000000000000100 //figure out why this has two
};

extern vr::IVRSystem* m_VRSystem;
extern vrinputemulator::VRInputEmulator inputEmulator;

extern uint32_t trackerIDs[];

extern glm::vec3 headPos, leftHandPos, rightHandPos;
extern glm::vec3 headPosReal, leftHandPosReal, rightHandPosReal;
extern glm::quat headRot, leftHandRot, rightHandRot;
extern glm::quat headRotReal, leftHandRotReal, rightHandRotReal;

extern glm::vec3 controllerPosOffset;
extern glm::quat controllerRotOffset;

extern bool trackersOverride;
extern bool active;

extern std::queue<uint8_t> buttonInputListener;

extern uint64_t inputButtonMask;
extern uint64_t pmButtonMask;

uint64_t GetButtonMaskFromConfig(QJsonObject config);
QJsonObject GetConfigFromButtonMask(uint64_t mask);

bool findTrackers();

uint32_t createTracker(std::string deviceName);
uint32_t createTracker();
uint32_t getTracker(std::string deviceName);

void GetTrackers();

void deleteVirtualDevice(int id);

void setVirtualDevicePosition(uint32_t id, glm::vec3 pos, glm::quat rot);
void setOffsetVirtualDevicePosition(uint32_t id, glm::vec3 pos, glm::quat rot);

void UpdateRealHardwarePositions();
void UpdateHardwarePositions();

//vr::TrackedControllerRole_LeftHand
vr::VRControllerState_t GetControllerState(vr::ETrackedControllerRole controller);

bool StartVR();
void DisconnectFromVRRuntime();