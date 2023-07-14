#include "cxxopts.hpp"
#include <iostream>
#include <algorithm>
#include <string>
#include <thread>
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

#ifdef _WIN32
#include <Windows.h>
#endif

#define PLAYSPACE_MOVER_VERSION "v0.1.7"

static vr::IVRSystem* m_VRSystem;
static vrinputemulator::VRInputEmulator inputEmulator;
static glm::mat4 offset;
static glm::mat4 lastOffset;
static int currentFrame;

static vr::TrackedDevicePose_t devicePoses[vr::k_unMaxTrackedDeviceCount];
// Stores the positions of each device for the current frame.
static glm::vec3 devicePos[vr::k_unMaxTrackedDeviceCount];
// Stores the positions of each device from the previous frame (So we can calculate deltas).
static glm::vec3 deviceLastPos[vr::k_unMaxTrackedDeviceCount];
// Stores the initial offset of the devices, so if the user adjusted them before app start we will respect it.
static glm::vec3 deviceBaseOffsets[vr::k_unMaxTrackedDeviceCount];
// List of virtual devices.
static std::vector<uint32_t> virtualDeviceIndexes;
// Current amount of acceleration (don't change this, it's gravity.)
static glm::vec3 acceleration;
// Current amount of velocity
static glm::vec3 velocity;
// How much force to apply in the opposite direction of movement while on the floor (0-10 are good values).
static float friction;
// How much force to apply in the opposite direction of movement while in the air (0-10 are good values).
static float airFriction;
// Time since last frame in seconds.
static float deltaTime;
// Keeps track if we need to apply impulse or not.
static bool appliedImpulse;
// Stores the previous frame of movement so we can use it to apply impulse on the next frame.
static glm::vec3 deltaMove;
// Sets if physics is enabled or not.
static bool physicsEnabled;
// Sets how hard you can throw yourself.
static float jumpMultiplier;
// Enables the ground or not.
static bool ground;
// Enables a fake tracker that tries to act like a hip.
static bool fakeTrackers;
static uint32_t hipID;
static uint32_t leftFootID;
static uint32_t rightFootID;
static float bodyHeight;
static bool orbitTracker;
static bool physicsToggleChanged;

void Help() {
    std::cout << "PlayspaceMover " << PLAYSPACE_MOVER_VERSION << "\n";
    std::cout << "\n";
    std::cout << "Copyright (C) 2017 Dalton Nell, PlayspaceMover Contributors\n";
    std::cout << "(https://github.com/naelstrof/VRPlayspaceMover/graphs/contributors)\n";
    std::cout << "Usage: VRPlayspaceMover [options]\n";
    std::cout << "\n";
    std::cout << "PlayspaceMover is an application that queries for a button press from\n";
    std::cout << "VR devices and moves the playspace based on it.\n";
    std::cout << "\n";
    std::cout << "-h, --help                    Print help and exit\n";
    std::cout << "-v, --version                 Print version and exit\n";
    std::cout << "Options\n";
    std::cout << "  -p, --physics               Enables physics.\n";
    std::cout << "  --noGround                  Disables ground collisions when physics is enabled.\n";
    std::cout << "  --fakeTrackers              Spawns some fake full body trackers that follow you.\n";
    std::cout << "  --orbitTracker              Fake trackers move relative to your HMD rotation.\n";
    std::cout << "  -l, --leftButtonMask=INT\n";
    std::cout << "                              Button mask that represents which button\n";
    std::cout << "                              to detect on the left controller as an integer.\n";
    std::cout << "                              (See Button Mappings for masks).\n";
    std::cout << "  -r, --rightButtonMask=INT\n";
    std::cout << "                              Button mask that represents which button\n";
    std::cout << "                              to detect on the right controller as an integer.\n";
    std::cout << "                              (See Button Mappings for masks).\n";
    std::cout << "  --leftTogglePhysicsMask=INT\n";
    std::cout << "                              Button mask that represents which buttons\n";
    std::cout << "                              need to be held on the left controller to toggle\n";
    std::cout << "                              physics.\n";
    std::cout << "                              (See Button Mappings for masks).\n";
    std::cout << "  --rightTogglePhysicsMask=INT\n";
    std::cout << "                              Button mask that represents which buttons\n";
    std::cout << "                              need to be held on the right controller to toggle\n";
    std::cout << "                              physics.\n";
    std::cout << "                              (See Button Mappings for masks).\n";
    std::cout << "  --resetButtonMask=INT\n";
    std::cout << "                              Button mask that represents which buttons\n";
    std::cout << "                              need to be held on BOTH controllers to reset\n";
    std::cout << "                              the playspace.\n";
    std::cout << "                              (See Button Mappings for masks).\n";
    std::cout << "  -g, --gravity=FLOAT\n";
    std::cout << "                              Sets how much gravity pulls down on you. Setting\n";
    std::cout << "                              this variable will also enable physics.\n";
    std::cout << "  -f, --friction=FLOAT\n";
    std::cout << "                              Sets how much friction slows you down when you're\n";
    std::cout << "                              on the ground. Higher friction means sharper stops.\n";
    std::cout << "                              Setting this variable will also enable physics.\n";
    std::cout << "  --airFriction=FLOAT\n";
    std::cout << "                              Sets how much friction slows you down when you're\n";
    std::cout << "                              in the air. Higher friction means sharper stops.\n";
    std::cout << "                              Setting this variable will also enable physics.\n";
    std::cout << "  -j, --jumpMultiplier=FLOAT\n";
    std::cout << "                              Multiplies the force applied to you when you throw\n";
    std::cout << "                              yourself. A good value would be around 0 and 100.\n";
    std::cout << "                              Setting this variable will also enable physics.\n";
    std::cout << "  --bodyHeight=FLOAT\n";
    std::cout << "                              Sets the height of your body for fake trackers.\n";
    std::cout << "                              Fake trackers are enabled and spawned at your feet\n";
    std::cout << "                              and in the middle of your body (hopefully your hips.)\n";
    std::cout << "Examples\n";
    std::cout << "    $ # Moves the playspace with ONLY A/X on Oculus.\n";
    std::cout << "    $ PlayspaceMover -l 128 -r 128\n";
    std::cout << "\n";
    std::cout << "Button Mappings\n";
    std::cout << "  We take as integers as a button mask, but they actually represent a bitmask.\n";
    std::cout << "  You'll have to exercise your CompSci brain to generate these masks. Each\n";
    std::cout << "  button is represented by a bit in a 32bit integer. Bit number 7 (1000000)\n";
    std::cout << "  would be 2^7, which is 128 as an integer. Button number 7 also happens to\n";
    std::cout << "  be the A and X buttons on the Oculus controllers. Therefore setting either\n";
    std::cout << "  button mask to `128` would make it so only the A or X button activated...\n";
    std::cout << "  Similarly, you can combine bits, so if you wanted button 2 and button 7\n";
    std::cout << "  to work with it, you could pass in `130` (2^2 + 2^7), then either would\n";
    std::cout << "  work!\n";
    std::cout << "  Below is a list of some known button masks (The mask is what you supply!).\n";
    std::cout << "    Oculus Masks    Button   Bit   Mask\n";
    std::cout << "                      A/X      7     128\n";
    std::cout << "                      B/Y      1     2\n";
    std::cout << "\n";
    std::cout << "    Vive Masks      Button   Bit   Mask\n";
    std::cout << "                      Menu     1     2\n";
    std::cout << "                      Grip     2     4\n";
    std::cout << "\n";
    std::cout << "Tips\n";
    std::cout << "    * Restarting the app resets your playspace!\n";
    std::cout << "    * VR Input Emulator has a log file that can be dozens of gigabytes if\n";
    std::cout << "you're on Oculus, it's in your SteamVR folder under drivers. Set it to\n";
    std::cout << "read-only to keep it from growing indefinitely.\n";
}

void updateVirtualDevices() {
    int count = inputEmulator.getVirtualDeviceCount();
    if (virtualDeviceIndexes.size() != count) {
        virtualDeviceIndexes.clear();
        for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; deviceIndex++) {
            try {
                virtualDeviceIndexes.push_back(inputEmulator.getVirtualDeviceInfo(deviceIndex).openvrDeviceId);
            }
            catch (vrinputemulator::vrinputemulator_exception e) {
                //skip
            }
        }
    }
}

bool isVirtualDevice(uint32_t deviceIndex) {
    if (virtualDeviceIndexes.empty()) { return false; }
    return std::find(virtualDeviceIndexes.begin(), virtualDeviceIndexes.end(), deviceIndex) != virtualDeviceIndexes.end();
}

void updatePositions() {
    float fSecondsSinceLastVsync;
    vr::VRSystem()->GetTimeSinceLastVsync(&fSecondsSinceLastVsync, NULL);
    float fDisplayFrequency = vr::VRSystem()->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);
    float fFrameDuration = 1.f / fDisplayFrequency;
    float fVsyncToPhotons = vr::VRSystem()->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SecondsFromVsyncToPhotons_Float);
    float fPredictedSecondsFromNow = fFrameDuration - fSecondsSinceLastVsync + fVsyncToPhotons;
    vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseRawAndUncalibrated, fPredictedSecondsFromNow, devicePoses, vr::k_unMaxTrackedDeviceCount);
    for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; deviceIndex++) {
        if (!vr::VRSystem()->IsTrackedDeviceConnected(deviceIndex)) {
            continue;
        }
        vr::TrackedDevicePose_t* pose = devicePoses + deviceIndex;
        vr::HmdMatrix34_t* poseMat = &(pose->mDeviceToAbsoluteTracking);
        if (pose->bPoseIsValid && pose->bDeviceIsConnected) {
            deviceLastPos[deviceIndex] = devicePos[deviceIndex];
            devicePos[deviceIndex] = glm::vec3(poseMat->m[0][3], poseMat->m[1][3], poseMat->m[2][3]);
        }
    }
    //vr::VRCompositor()->WaitGetPoses(devicePoses, vr::k_unMaxTrackedDeviceCount, NULL, 0);

    /*vr::VRCompositor()->SetTrackingSpace(vr::TrackingUniverseRawAndUncalibrated);
    vr::VRCompositor()->WaitGetPoses(devicePoses, vr::k_unMaxTrackedDeviceCount, NULL, 0);
    for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; deviceIndex++) {
        if (!vr::VRSystem()->IsTrackedDeviceConnected(deviceIndex)) {
            continue;
        }
        vr::TrackedDevicePose_t* pose = devicePoses + deviceIndex;
        vr::HmdMatrix34_t* poseMat = &(pose->mDeviceToAbsoluteTracking);
        if (pose->bPoseIsValid && pose->bDeviceIsConnected) {
            deviceLastPos[deviceIndex] = devicePos[deviceIndex];
            devicePos[deviceIndex] = glm::vec3(poseMat->m[0][3], poseMat->m[1][3], poseMat->m[2][3]);
        }
    }*/
}

// Checks that every individual button in mask is pressed, when compared to button.
bool checkAll(uint64_t button, uint64_t mask) {
    bool ret = mask ? 1 : 0;
    for (int i = 0; i < 64; i++) {
        if (mask >> i & 1) {
            ret = ret && (button & (mask & ((uint64_t)1 << i)));
        }
    }
    return ret;
}

void updateOffset(unsigned long long leftButtonMask, unsigned long long  rightButtonMask, unsigned long long resetButtonMask, unsigned long long  leftTogglePhysicsMask, unsigned long long  rightTogglePhysicsMask) {
    vr::VRControllerState_t leftButtons, rightButtons;
    leftButtons.ulButtonPressed = 0;
    rightButtons.ulButtonPressed = 0;
    glm::vec3 delta = glm::vec3(0, 0, 0);
    float count = 0.f;
    auto leftId = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
    if (leftId != vr::k_unTrackedDeviceIndexInvalid) {
        vr::VRSystem()->GetControllerState(leftId, &leftButtons, sizeof(vr::VRControllerState_t));
        if (leftButtons.ulButtonPressed & leftButtonMask) {
            delta += devicePos[leftId] - deviceLastPos[leftId];
            count++;
        }
    }
    auto rightId = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
    if (rightId != vr::k_unTrackedDeviceIndexInvalid) {
        vr::VRSystem()->GetControllerState(rightId, &rightButtons, sizeof(vr::VRControllerState_t));
        if (rightButtons.ulButtonPressed & rightButtonMask) {
            delta += devicePos[rightId] - deviceLastPos[rightId];
            count++;
        }
    }

    if (count) {
        delta /= count;
        velocity = glm::vec3(0);
        appliedImpulse = false;
    }
    else {
        if (!appliedImpulse) {
            velocity += deltaMove * jumpMultiplier / deltaTime / 90.f;
            appliedImpulse = true;
        }
        velocity += acceleration * deltaTime;
    }
    if (checkAll(leftButtons.ulButtonPressed, leftTogglePhysicsMask) || checkAll(rightButtons.ulButtonPressed, rightTogglePhysicsMask)) {
        if (!physicsToggleChanged) {
            physicsEnabled = !physicsEnabled;
            physicsToggleChanged = true;
        }
    }
    else {
        physicsToggleChanged = false;
    }
    // Only apply physics if it's enabled.
    if (physicsEnabled) {
        delta += velocity * deltaTime;
    }
    //delta = glm::clamp(delta, glm::vec3(-0.1f), glm::vec3(0.1f));
    deltaMove = delta;

    lastOffset = offset;
    if (checkAll(leftButtons.ulButtonPressed, resetButtonMask) && checkAll(rightButtons.ulButtonPressed, resetButtonMask)) {
        offset = glm::mat4(1);
        velocity = glm::vec3(0);
        appliedImpulse = true;
    }
    else {
        offset = glm::translate(offset, -delta);
    }
    //offset = glm::rotate(offset, 0.001f, glm::vec3(0, 1, 0));
}

void collide() {
    // We only collide if we have physics enabled...
    if (!physicsEnabled) {
        return;
    }
    glm::vec3 positionOffset = (offset * glm::vec4(0, 0, 0, 1)).xyz();
    // If we're falling into the floor
    if (positionOffset.y <= 0 && ground) {
        // Push us back up above the floor
        offset = glm::translate(offset, glm::vec3(0, -positionOffset.y, 0));
        velocity.y = 0;
        // Apply friction
        float ratio = 1.f / (1.f + (deltaTime * friction));
        velocity *= ratio;
    }
    else {
        float ratio = 1.f / (1.f + (deltaTime * airFriction));
        velocity *= ratio;
    }
}

void move() {
    for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; deviceIndex++) {
        if (!vr::VRSystem()->IsTrackedDeviceConnected(deviceIndex)) {
            continue;
        }
        glm::vec3 oldpos = (glm::inverse(lastOffset) * glm::vec4(devicePos[deviceIndex], 1.f)).xyz();
        glm::vec3 newpos = (offset * glm::vec4(oldpos, 1.f)).xyz();
        devicePos[deviceIndex] = newpos;
        glm::vec3 delta = deviceBaseOffsets[deviceIndex];
        // Virtual devices need to be moved half as much, don't ask me why
        if (isVirtualDevice(deviceIndex)) {
            delta += (newpos - oldpos) * .5f;
        }
        else {
            delta += (newpos - oldpos);
        }
        vr::HmdVector3d_t copy;
        copy.v[0] = delta.x;
        copy.v[1] = delta.y;
        copy.v[2] = delta.z;
        inputEmulator.enableDeviceOffsets(deviceIndex, true, false);
        inputEmulator.setWorldFromDriverTranslationOffset(deviceIndex, copy, false);

        // Rotations on Oculus HMD's are bugged atm, so this is a no-no currently.
        //glm::fquat quat = glm::quat_cast(offset);
        //vr::HmdQuaternion_t quatCopy;
        //quatCopy.w = quat.w;
        //quatCopy.x = quat.x;
        //quatCopy.y = quat.y;
        //quatCopy.z = quat.z;
        //inputEmulator.setDriverRotationOffset(deviceIndex, quatCopy, false);
    }
}

void updateBaseOffsets() {
    for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; deviceIndex++) {
        if (!vr::VRSystem()->IsTrackedDeviceConnected(deviceIndex)) {
            deviceBaseOffsets[deviceIndex] = glm::vec3(0);
            continue;
        }
        vrinputemulator::DeviceOffsets data;
        try {
            inputEmulator.getDeviceOffsets(deviceIndex, data);
            glm::vec3 offset;
            offset.x = (float)data.worldFromDriverTranslationOffset.v[0];
            offset.y = (float)data.worldFromDriverTranslationOffset.v[1];
            offset.z = (float)data.worldFromDriverTranslationOffset.v[2];
            deviceBaseOffsets[deviceIndex] = offset;
        }
        catch (vrinputemulator::vrinputemulator_notfound e) {
            glm::vec3 offset;
            offset.x = 0;
            offset.y = 0;
            offset.z = 0;
            deviceBaseOffsets[deviceIndex] = offset;
        }
    }
}

void setVirtualDevicePosition(uint32_t id, glm::vec3 pos, glm::quat rot) {
    vr::DriverPose_t pose = inputEmulator.getVirtualDevicePose(id);
    pose.vecPosition[0] = pos.x;
    pose.vecPosition[1] = pos.y;
    pose.vecPosition[2] = pos.z;
    pose.poseIsValid = true;
    pose.deviceIsConnected = true;
    pose.result = vr::TrackingResult_Running_OK;
    pose.qRotation.w = rot.w;
    pose.qRotation.x = rot.x;
    pose.qRotation.y = rot.y;
    pose.qRotation.z = rot.z;
    inputEmulator.setVirtualDevicePose(id, pose, false);
}

void updateFakeTrackers() {
    if (!fakeTrackers) {
        return;
    }
    uint32_t HMDID = vr::k_unTrackedDeviceIndexInvalid;
    for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; deviceIndex++) {
        if (!vr::VRSystem()->IsTrackedDeviceConnected(deviceIndex)) {
            continue;
        }
        if (vr::VRSystem()->GetTrackedDeviceClass(deviceIndex) == vr::TrackedDeviceClass_HMD) {
            HMDID = deviceIndex;
            break;
        }
    }
    if (HMDID == vr::k_unTrackedDeviceIndexInvalid) {
        return;
    }

    vr::TrackedDevicePose_t* hmdPose = devicePoses + HMDID;
    glm::mat3x4* hmdMat = (glm::mat3x4*)&(hmdPose->mDeviceToAbsoluteTracking);
    glm::mat3x3 hmdRotMat = glm::mat3x3(*hmdMat);
    if (hmdPose->bPoseIsValid && hmdPose->bDeviceIsConnected) {
        glm::vec3 realHMDPos = (glm::inverse(offset) * glm::vec4(devicePos[HMDID], 1)).xyz();
        glm::quat hmdRot = glm::inverse(glm::quat_cast(hmdRotMat));

        glm::vec3 down;
        glm::quat trackersRot;
        glm::vec3 footRight;
        glm::vec3 footForward;

        if (orbitTracker) {
            down = hmdRot * glm::vec3(0, -1, 0);
            trackersRot = hmdRot;
            footRight = trackersRot * glm::vec3(1, 0, 0);
            footForward = trackersRot * glm::vec3(0, 0, 1);
        }
        else {
            down = glm::vec3(0, -1, 0);
            glm::vec3 headRight = hmdRot * glm::vec3(1, 0, 0);
            headRight.y = 0;
            trackersRot = glm::quatLookAt(glm::normalize(headRight), glm::vec3(0, 1, 0));
            footRight = trackersRot * glm::vec3(0, 0, 1);
            footForward = trackersRot * glm::vec3(-1, 0, 0);
        }
        glm::vec3 hipPos = realHMDPos + down * (bodyHeight / 2.f);
        glm::vec3 leftFootPos = realHMDPos + down * bodyHeight + (footForward - footRight) * 0.17f;
        glm::vec3 rightFootPos = realHMDPos + down * bodyHeight + (footForward + footRight) * 0.17f;
        setVirtualDevicePosition(hipID, hipPos, trackersRot);
        setVirtualDevicePosition(leftFootID, leftFootPos, trackersRot);
        setVirtualDevicePosition(rightFootID, rightFootPos, trackersRot);
    }
}

bool findTrackers() {
    if (inputEmulator.getVirtualDeviceCount() == 3) {
        for (int i = 0; i < 3; i++) {
            vr::DriverPose_t pose = inputEmulator.getVirtualDevicePose(i);
            if (pose.deviceIsConnected == true || pose.result != vr::TrackingResult_Uninitialized || pose.poseIsValid == true) {
                return false;
            }
        }
        hipID = 0;
        leftFootID = 1;
        rightFootID = 2;
        return true;
    }
    return false;
}

uint32_t createTracker() {
    uint32_t id = inputEmulator.getVirtualDeviceCount();
    inputEmulator.addVirtualDevice(vrinputemulator::VirtualDeviceType::TrackedController, std::to_string(id), false);
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_TrackingSystemName_String, "lighthouse");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_ModelNumber_String, "Vive Controller MV");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_RenderModelName_String, "vr_controller_vive_1_5");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_WillDriftInYaw_Bool, false);
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_ManufacturerName_String, "HTC");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_TrackingFirmwareVersion_String, "1465809478 htcvrsoftware@firmware-win32 2016-06-13 FPGA 1.6/0/0 VRC 1465809477 Radio 1466630404");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_HardwareRevision_String, "product 129 rev 1.5.0 lot 2000/0/0 0");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_DeviceIsWireless_Bool, true);
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_HardwareRevision_Uint64, (uint64_t)2164327680);
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_FirmwareVersion_Uint64, (uint64_t)1465809478);
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_DeviceClass_Int32, 3);
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_SupportedButtons_Uint64, (uint64_t)12884901895);
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_Axis0Type_Int32, 1);
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_Axis1Type_Int32, 3);
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_Axis2Type_Int32, 0);
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_Axis3Type_Int32, 0);
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_Axis4Type_Int32, 0);
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_IconPathName_String, "icons");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceOff_String, "{htc}controller_status_off.png");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceSearching_String, "{htc}controller_status_searching.gif");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceSearchingAlert_String, "{htc}controller_status_alert.gif");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceReady_String, "{htc}controller_status_ready.png");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceNotReady_String, "{htc}controller_status_error.png");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceStandby_String, "{htc}controller_status_standby.png");
    inputEmulator.setVirtualDeviceProperty(id, vr::ETrackedDeviceProperty::Prop_NamedIconPathDeviceAlertLow_String, "{htc}controller_status_ready_low.png");
    inputEmulator.publishVirtualDevice(id);
    return id;
}

void deleteVirtualDevice(int id) {
    vr::DriverPose_t pose = inputEmulator.getVirtualDevicePose(id);
    pose.deviceIsConnected = false;
    pose.result = vr::TrackingResult_Uninitialized;
    pose.poseIsValid = false;
    inputEmulator.setVirtualDevicePose(id, pose, false);
}

void onClose() {
    if (fakeTrackers) {
        deleteVirtualDevice(hipID);
        deleteVirtualDevice(leftFootID);
        deleteVirtualDevice(rightFootID);
    }
    offset = glm::mat4x4(1);
    velocity = glm::vec3(0);
    move();
    inputEmulator.disconnect();
}

void signalHandler(int signum) {
    std::cerr << "Interrupt signal (" << signum << ") received, cleaning up...\n" << std::flush;
    onClose();
    exit(signum);
}

#ifdef _WIN32
BOOL WINAPI ConsoleHandler(DWORD CEvent) {
    std::cerr << "Console window was closed, cleaning up...\n" << std::flush;
    if (CEvent == CTRL_CLOSE_EVENT) {
        onClose();
    }
    return TRUE;
}
#endif

int app(int argc, const char** argv) {
    cxxopts::Options options("PlayspaceMover", "Lets you grab your playspace and move it.");
    options.add_options()
        ("h,help", "Prints help.")
        ("v,version", "Prints version.")
        ("l,leftButtonMask", "Specifies the buttons that trigger the playspace grab.", cxxopts::value<unsigned long long >()->default_value("130"))
        ("r,rightButtonMask", "Specifies the buttons that trigger the playspace grab.", cxxopts::value<unsigned long long >()->default_value("130"))
        ("leftTogglePhysicsMask", "Specifies the buttons that toggle physics.", cxxopts::value<unsigned long long >()->default_value("0"))
        ("rightTogglePhysicsMask", "Specifies the buttons that toggle physics.", cxxopts::value<unsigned long long >()->default_value("0"))
        ("resetButtonMask", "Specifies the buttons that trigger a playspace reset.", cxxopts::value<unsigned long long >()->default_value("0"))
        ("g,gravity", "Sets how intense gravity is in meters.", cxxopts::value<float>()->default_value("9.81"))
        ("f,friction", "Sets how much friction the ground applies. (Try values from 0 to 10.)", cxxopts::value<float>()->default_value("8"))
        ("airFriction", "Sets how much friction the air applies. (Try values from 0 to 10.)", cxxopts::value<float>()->default_value("0"))
        ("noGround", "Disables the ground, only useful if physics is enabled.")
        ("j,jumpMultiplier", "Sets how hard you can throw yourself. (Try values from 0 to 100.)", cxxopts::value<float>()->default_value("80"))
        ("p,physics", "Enables physics with the default settings.")
        ("fakeTrackers", "Enables a bunch of fake devices that act like full body tracking.")
        ("bodyHeight", "Sets the distance between the head and feet, in meters.", cxxopts::value<float>()->default_value("2"))
        ("orbitTracker", "Fake Trackers move relative to HMD rotation.")
        ("positional", "Positional parameters", cxxopts::value<std::vector<std::string>>())
        ;

    options.parse_positional("positional");
    auto result = options.parse(argc, argv);

    // We don't support any positional arguments, so consider any positional arguments as a malformed command.
    if (result.count("positional") > 0) {
        auto& positional = result["positional"].as<std::vector<std::string>>();
        throw cxxopts::OptionException("Unknown positional parameter `" + positional[0] + "` supplied.");
    }

    if (result["help"].as<bool>()) {
        Help();
        return 0;
    }
    if (result["version"].as<bool>()) {
        std::cout << PLAYSPACE_MOVER_VERSION << "\n";
        return 0;
    }

    // Initialize stuff
    vr::EVRInitError error = vr::VRInitError_Compositor_Failed;
    std::cout << "Looking for SteamVR..." << std::flush;
    while (error != vr::VRInitError_None) {
        m_VRSystem = vr::VR_Init(&error, vr::VRApplication_Overlay);
        if (error != vr::VRInitError_None) {
            std::cout << "\nFailed due to reason " << VR_GetVRInitErrorAsSymbol(error) << "\n" << std::flush;
            std::cout << "Trying again in a few seconds...\n" << std::flush;
            std::this_thread::sleep_for(std::chrono::seconds(4));
        }
    }
    std::cout << "Success!\n";
    std::cout << "Looking for VR Input Emulator..." << std::flush;
    while (true) {
        try {
            inputEmulator.connect();
            break;
        }
        catch (vrinputemulator::vrinputemulator_connectionerror e) {
            std::cout << "\nFailed to connect to open vr input emulator, ensure you've installed it. If you have, try running this fix: https://drive.google.com/open?id=1Gn3IOm6GbkINplbEenu0zTr3DkB1E8Hc \n" << std::flush;
            std::this_thread::sleep_for(std::chrono::seconds(4));
            continue;
        }
    }
    std::cout << "Success!\n";

    acceleration = glm::vec3(0, result["gravity"].as<float>(), 0);
    velocity = glm::vec3(0);
    jumpMultiplier = result["jumpMultiplier"].as<float>();
    airFriction = result["airFriction"].as<float>();
    friction = result["friction"].as<float>();
    ground = !result["noGround"].as<bool>();
    bodyHeight = result["bodyHeight"].as<float>();
    orbitTracker = result["orbitTracker"].as<bool>();
    fakeTrackers = result["fakeTrackers"].as<bool>() || orbitTracker || result.count("bodyHeight");
    if (fakeTrackers && !findTrackers()) {
        hipID = createTracker();
        leftFootID = createTracker();
        rightFootID = createTracker();
    }
    // Only enable physics if they specify one of the variables...
    physicsEnabled = (result.count("jumpMultiplier") || result.count("airFriction") || result.count("friction") || result.count("noGround") || result.count("gravity") || result["physics"].as<bool>());
    offset = glm::mat4x4(1);
    lastOffset = offset;

    unsigned long long  leftButtonMask = result["leftButtonMask"].as<unsigned long long >();
    unsigned long long  rightButtonMask = result["rightButtonMask"].as<unsigned long long >();
    unsigned long long  resetButtonMask = result["resetButtonMask"].as<unsigned long long >();

    unsigned long long  rightTogglePhysicsButtonMask = result["rightTogglePhysicsMask"].as<unsigned long long >();
    unsigned long long  leftTogglePhysicsButtonMask = result["leftTogglePhysicsMask"].as<unsigned long long >();

    updateBaseOffsets();
#ifdef _WIN32
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE) {
        std::cerr << "Failed to set console handler, I won't shut down properly!\n";
    }
#endif
    signal(SIGINT, signalHandler);
    std::cout << "READY! Press CTRL+C with this window focused to quit.\n" << std::flush;

    // Main loop
    bool running = true;
    appliedImpulse = true;
    auto lastTime = std::chrono::high_resolution_clock::now();
    int numFramePresents = 0;
    while (running) {
        if (vr::VRCompositor() != NULL) {
            vr::Compositor_FrameTiming t;
            t.m_nSize = sizeof(vr::Compositor_FrameTiming);
            bool hasFrame = vr::VRCompositor()->GetFrameTiming(&t, 0);
            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> dt = currentTime - lastTime;
            deltaTime = dt.count();
            // If the frame has changed we update, if a frame was redisplayed we update.
            if ((hasFrame && currentFrame != t.m_nFrameIndex) || (hasFrame && t.m_nNumFramePresents != numFramePresents)) {
                currentFrame = t.m_nFrameIndex;
                numFramePresents = t.m_nNumFramePresents;
                lastTime = currentTime;

                updateVirtualDevices();
                updatePositions();
                updateOffset(leftButtonMask, rightButtonMask, resetButtonMask, leftTogglePhysicsButtonMask, rightTogglePhysicsButtonMask);
                updateFakeTrackers();
                collide();
                move();

                /*vr::ETrackedPropertyError errProp;
                int microsecondWait;
                float flDisplayFrequency = vr::VRSystem()->GetFloatTrackedDeviceProperty(0, vr::Prop_DisplayFrequency_Float, &errProp);
                if (flDisplayFrequency) {
                    float flSecondsPerFrame = 1.0f / flDisplayFrequency;
                    microsecondWait = (int)(flSecondsPerFrame * 1000.f * 1000.f);
                } else {
                    microsecondWait = (int)(t.m_flCompositorIdleCpuMs*1000.f);
                }
                std::this_thread::sleep_for(std::chrono::microseconds(glm::clamp(microsecondWait, 11111, 22222)));*/

                // Sleep for just under 1/90th of a second, so that maybe the next frame will be available.
                std::this_thread::sleep_for(std::chrono::microseconds(10000));
            }
            else {
                // Still waiting on the next frame, wait less this time.
                std::this_thread::sleep_for(std::chrono::microseconds(1111));
            }
        }
    }
    return 0;
}

int main(int argc, const char** argv) {
    try {
        return app(argc, argv);
    }
    catch (cxxopts::OptionException& e) {
        std::cerr << "Parse error: " << e.what() << "\n";
        std::cerr << "Try using the --help parameter!\n" << std::flush;
    }
    return 1;
}