#pragma once

#include "vrUtil.h"
#include "OverlayManager.h"
#include "DashboardWidget.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <string>
#include <iostream>

class PoseTracker {
public:
	static void SetPose(uint8_t camera, QJsonObject poseData);
	static void InitTrackers();

	void Init(uint8_t trackerIndex);

	void InitCamera(uint8_t camera);
	void ClearPose(uint8_t camera);
	void UpdatePose(uint8_t camera, float x, float y, float score);
	glm::vec2 GetPose(uint8_t camera);

	uint8_t CalculatePosition();
	bool CalculateSingleCameraPosition();
	uint8_t CalculateOrientation();

	uint8_t getNumberOfCams();
	
	glm::vec3 position;
	glm::quat rotation;

	glm::vec3 hipRightRealPos;
	bool hipPosValid = false;

	bool hasValidPosition = false;
	bool hasAmbiguousPosition = false;
	bool hasDualPosition = false;
	glm::vec3 amb1, amb2;
private:
	uint8_t tracker;

	glm::vec2 points[16];
	float scores[16];
	glm::vec3 directions[16];

	
	uint16_t cameraFlags = 0;

	void CalculateMultiPosition();
	void UpdateDirections();

	uint8_t CalculateOneValid(glm::vec3 center, float r, glm::vec3 pos, glm::vec3 dir);
	uint8_t CalculateTwoValid(glm::vec3 p1, float r1, glm::vec3 p2, float r2, glm::vec3 pos, glm::vec3 dir);

	void CalculateSingleAnkle(uint8_t n, uint8_t knee, uint8_t hip);
	void CalculateSingleKnee(uint8_t n, uint8_t ankle, uint8_t hip);
	void CalculateSingleElbow(uint8_t n, uint8_t wrist, uint8_t shoulder);

	void CalculateOrientationAnkle(uint8_t knee, uint8_t hip);

	static glm::vec3 GetFootForward(uint8_t foot, uint8_t knee, uint8_t hip);
};

struct Camera {
	float radPerPixel;
	glm::vec3 position;
	glm::quat rotation;
	glm::mat4x4 transform;
	bool active = false;
	bool connected = false;
	bool waitingForSize = true;
	uint16_t width, height;
	uint8_t index;

	static void SetSize(uint8_t index, uint16_t width, uint16_t height);
	static void OnConnect(uint8_t index);
	static void OnStart(uint8_t index);

	void Init(uint8_t index);
	void Calibrate(glm::vec3 position, glm::quat qp,
		glm::vec3 v1, glm::vec2 p1, glm::quat q1,
		glm::vec3 v2, glm::vec2 p2, glm::quat q2,
		glm::vec3 v3, glm::vec2 p3, glm::quat q3);
	void CalibrateDistances(glm::vec3 v1, glm::quat q1, glm::vec3 v2, glm::quat q2, glm::vec3 v3);

	glm::vec3 GetVector(glm::vec2 coords);
};


extern glm::vec3 handToWrist;
extern float shoulderToShoulder,
shoulderToHip, hipWidth,
upperArmLen, lowerArmLen,
upperLegLen, lowerLegLen; //add shoulder to head

extern Camera cameras[];
extern PoseTracker trackers[];

const std::string PoseNames[17] = {
	"nose",
	"left_eye",
	"right_eye",
	"left_ear",
	"right_ear",
	"left_shoulder",
	"right_shoulder",
	"left_elbow",
	"right_elbow",
	"left_wrist",
	"right_wrist",
	"left_hip",
	"right_hip",
	"left_knee",
	"right_knee",
	"left_ankle",
	"right_ankle"
};

enum Poses : uint8_t {
	nose,
	left_eye,
	right_eye,
	left_ear,
	right_ear,
	left_shoulder,
	right_shoulder,
	left_elbow,
	right_elbow,
	left_wrist,
	right_wrist,
	left_hip,
	right_hip,
	left_knee,
	right_knee,
	left_ankle,
	right_ankle
};

const bool PoseTrackers[17] = {
	false,	//       nose
	false,	// left  eye
	false,	// right eye
	false,	// left  ear
	false,	// right ear
	false,	// left  shoulder
	false,	// right shoulder
	false,	// left  elbow
	false,	// right elbow
	false,	// left  wrist
	false,	// right wrist
	false,	// left  hip
	true,	// right hip
	false,	// left  knee
	false,	// right knee
	true,	// left  ankle
	true	// right ankle
};