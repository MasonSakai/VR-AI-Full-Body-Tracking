#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>
#include <iostream>

class PoseTracker {
public:

	void Init(uint8_t trackerIndex);

	void InitCamera(uint8_t camera);
	void ClearPose(uint8_t camera);
	void UpdatePose(uint8_t camera, float x, float y, float score);
	glm::vec2 GetPose(uint8_t camera);

	uint8_t CalculatePosition();
	bool CalculateSingleCameraPosition(PoseTracker *trackers);
	uint8_t CalculateOrientation(PoseTracker* trackers);
	
	glm::vec3 position;
	glm::quat rotation;
private:
	uint8_t tracker;

	glm::vec2 points[16];
	float scores[16];
	glm::vec3 directions[16];

	
	uint16_t cameraFlags = 0;
	uint8_t getNumberOfCams();

	void CalculateMultiPosition();
	void UpdateDirections();
};

struct Camera {
	float radPerPixel;
	glm::vec3 position;
	glm::quat rotation;
	bool active = false;
	bool connected = false;
	bool waitingForSize = false;
	uint16_t width, height;

	void Calibrate(glm::vec3 position, glm::quat qp,
		glm::vec3 v1, glm::vec2 p1, glm::quat q1,
		glm::vec3 v2, glm::vec2 p2, glm::quat q2,
		glm::vec3 v3, glm::vec2 p3, glm::quat q3);

	glm::vec3 GetVector(glm::vec2 coords);
};

static Camera cameras[16];
static PoseTracker trackers[17];

static glm::vec3 handToWrist;

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