#pragma once

#include "openvr.h"
#include "vrinputemulator.h"
#include "mathUtil.h"
#include "PlayspaceMover.h"

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <queue>;

extern vr::IVRSystem* m_VRSystem;
extern vrinputemulator::VRInputEmulator inputEmulator;

extern glm::vec3 headPos, leftHandPos, rightHandPos;
extern glm::vec3 headPosReal, leftHandPosReal, rightHandPosReal;
extern glm::quat headRot, leftHandRot, rightHandRot;
extern glm::quat headRotReal, leftHandRotReal, rightHandRotReal;

extern bool trackersOverride;
extern bool active;

extern bool FlagNoVR;

extern std::queue<uint8_t> buttonInputListener;


//update
bool findTrackers();

uint32_t createTracker(const char* deviceName);
uint32_t createTracker();

void deleteVirtualDevice(int id);

void setVirtualDevicePosition(uint32_t id, glm::vec3 pos, glm::quat rot);
void setOffsetVirtualDevicePosition(uint32_t id, glm::vec3 pos, glm::quat rot);

void EnableHardwareOffset();
void UpdateHardwareOffset();
void DisableHardwareOffset();

void UpdateRealHardwarePositions();
void UpdateHardwarePositions();

//vr::TrackedControllerRole_LeftHand
//Fix in NoVR
vr::VRControllerState_t GetControllerState(vr::ETrackedControllerRole controller);

bool StartVR();

enum ButtonMasks : uint64_t {
	OculusBY      = 0b00000000000000000000000000000000010,
	OculusAX      = 0b00000000000000000000000000010000000,
	OculusStick   = 0b00100000000000000000000000000000000,
	OculusTrigger = 0b01000000000000000000000000000000000,
	OculusBumper  = 0b10000000000000000000000000000000100 //figure out why this has two
};