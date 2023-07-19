#include "PoseTracker.h"

const float handToWristRatio = .88;
bool distancesRecorded = false;
float shoulderToShoulder,
shoulderToHip, hipWidth,
upperArmLen, lowerArmLen,
upperLegLen, lowerLegLen;

Camera cameras[16];
PoseTracker trackers[17];

glm::vec3 handToWrist;

void PoseTracker::Init(uint8_t trackerIndex) {
	tracker = trackerIndex;

	cameraFlags = 0;
}

void PoseTracker::InitCamera(uint8_t camera) {
	cameraFlags &= ~(1 << camera);
	points[camera] = glm::vec2();
	scores[camera] = 0;
}
void PoseTracker::ClearPose(uint8_t camera) {
	cameraFlags &= ~(1 << camera);
}
void PoseTracker::UpdatePose(uint8_t camera, float x, float y, float score) {
	if (cameras[camera].active) cameraFlags |= (1 << camera);
	else cameraFlags &= ~(1 << camera);
	points[camera].x = x;
	points[camera].y = y;
	scores[camera] = score;
}
glm::vec2 PoseTracker::GetPose(uint8_t camera) {
	return points[camera];
}

uint8_t PoseTracker::getNumberOfCams() {
	uint8_t n = 0;
	for (uint8_t i = 0; i < 16; i++) {
		if (cameraFlags & (1 << i)) n++;
	}
	return n;
}

glm::vec3 CalculateClosest(glm::vec3 p1, glm::vec3 v1, glm::vec3 p2, glm::vec3 v2) {
	glm::vec3 del = p2 - p1;

	glm::vec3 n = glm::cross(v1, v2);
	glm::vec3 n1 = glm::cross(v1, n);
	glm::vec3 n2 = glm::cross(v2, n);

	glm::vec3 c1 = p1 + v1 * (glm::dot(del, n2) / glm::dot(v1, n2));
	glm::vec3 c2 = p2 + v2 * (glm::dot(-del, n1) / glm::dot(v2, n1));

	return (c1 + c2) / 2.0f;
}


void PoseTracker::UpdateDirections() {
	for (int i = 0; i < 16; i++) {
		if (cameraFlags & (1 << i)) {
			directions[i] = cameras[i].GetVector(points[i]);
		}
	}
}

void PoseTracker::CalculateMultiPosition() {
	glm::vec3 pos;
	float total = 0;

	for (uint8_t i = 0; i < 15; i++) {
		if (!(cameraFlags & (1 << i))) continue;
		for (uint8_t j = i + 1; j < 16; j++) {
			if (!(cameraFlags & (1 << j))) continue;
			float weight = scores[i] * scores[j];
			total += weight;
			pos += weight * CalculateClosest(cameras[i].position, directions[i], cameras[j].position, directions[j]);
		}
	}
	position = pos / total;
}

uint8_t PoseTracker::CalculatePosition() {
	uint8_t n = getNumberOfCams();
	if (n == 0) return 0;
	UpdateDirections();
	if (n == 1) return 1;
	CalculateMultiPosition();
	return 2;
}

//when calculating foot orientation,
//use floor when on or below
//put toes on when near (<60/70 deg)
//use leg when above


//return true if able to make pose
bool PoseTracker::CalculateSingleCameraPosition(PoseTracker* trackers) {
	uint8_t n = 0;
	for (n = 0; n < 17; n++)
		if (cameraFlags & (1 << n))
			break;
	if (n == 16) return false;

	position = cameras[n].position + directions[n] * 10.0f;

	return true;
}

// _1 (left) and _2 (right) are from T-pose
// _3 is next to foot
// position is against the camera
// qp is against camera, q3 is against ground, used to correct the camera's position
void Camera::Calibrate(glm::vec3 position, glm::quat qp,
	glm::vec3 v1, glm::vec2 p1, glm::quat q1,
	glm::vec3 v2, glm::vec2 p2, glm::quat q2,
	glm::vec3 v3, glm::vec2 p3, glm::quat q3) {

	std::cout << "pos: {" << position.x << ", " << position.y << ", " << position.z << "}\n";
	std::cout << "v1: {" << v1.x << ", " << v1.y << ", " << v1.z << "}\n";
	std::cout << "p1: {" << p1.x << ", " << p1.y << "}\n";
	std::cout << "v2: {" << v2.x << ", " << v2.y << ", " << v2.z << "}\n";
	std::cout << "p2: {" << p2.x << ", " << p2.y << "}\n";
	std::cout << "v3: {" << v3.x << ", " << v3.y << ", " << v3.z << "}\n";
	std::cout << "p3: {" << p3.x << ", " << p3.y << "}\n\n" << std::flush;

	//correct camera position
	glm::vec3 cameraPosOffset = glm::rotate(glm::inverse(q3), glm::vec3(0, -v3.y, 0));
	position += glm::rotate(qp, cameraPosOffset);
	this->position = position;
	std::cout << "cameraPosOffset: {" << cameraPosOffset.x << ", " << cameraPosOffset.y << ", " << cameraPosOffset.z << "}\n";
	std::cout << "pos: {" << position.x << ", " << position.y << ", " << position.z << "}\n\n" << std::flush;

	//Get wrist real positions
	glm::vec3 handToHandDelta = v2 - v1;
	glm::vec3 handToHandCenter = (v1 + v2) / 2.0f;
	glm::vec3 wristLeftReal = handToHandCenter - handToHandDelta * handToWristRatio * .5f;
	glm::vec3 wristRightReal = handToHandCenter + handToHandDelta * handToWristRatio * .5f;

	//correct wrist position with hand to wrist ratio
	glm::vec3 handToWristLeft = wristLeftReal - v1;
	glm::vec3 handToWristRight = wristRightReal - v2;
	std::cout << "handToWristLeft:  {" << handToWristLeft.x << ", " << handToWristLeft.y << ", " << handToWristLeft.z << "}\n";
	std::cout << "handToWristRight: {" << handToWristRight.x << ", " << handToWristRight.y << ", " << handToWristRight.z << "}\n";
	//get wrist offsets using q1 and q2
	handToWristLeft = glm::rotate(glm::inverse(q1), handToWristLeft);
	handToWristRight = glm::rotate(glm::inverse(q2), handToWristRight);
	handToWrist = (handToWristLeft + handToWristRight) / 2.0f;
	std::cout << "handToWristLeft:  {" << handToWristLeft.x << ", " << handToWristLeft.y << ", " << handToWristLeft.z << "}\n";
	std::cout << "handToWristRight: {" << handToWristRight.x << ", " << handToWristRight.y << ", " << handToWristRight.z << "}\n";
	std::cout << "handToWrist:      {" << handToWrist.x << ", " << handToWrist.y << ", " << handToWrist.z << "}\n\n" << std::flush;

	//get wrist3 position
	glm::vec3 wrist3Real = v3 + glm::rotate(q3, handToWrist);
	std::cout << "v3:         {" << v3.x << ", " << v3.y << ", " << v3.z << "}\n";
	std::cout << "wrist3Real: {" << wrist3Real.x << ", " << wrist3Real.y << ", " << wrist3Real.z << "}\n\n" << std::flush;

	//get radPerPixel
	glm::vec3 wLRDel = wristLeftReal - position;
	glm::vec3 wRRDel = wristRightReal - position;
	float wtwRad = acosf(glm::dot(wLRDel, wRRDel) / (glm::length(wLRDel) * glm::length(wRRDel)));
	float wtwPix = glm::length(p2 - p1);
	radPerPixel = wtwRad / wtwPix;
	std::cout << "radPerPixel: " << radPerPixel << "\n\n" << std::flush;

	//get center in 3d space
	glm::vec2 center = glm::vec2(width, height) / 2.0f;
	float proj1 = glm::dot(center, p2 - p1) / glm::length2(p3 - p1);
	float proj2 = glm::dot(center, p3 - p1) / glm::length2(p3 - p1);
	glm::vec3 lerp1 = lerp(wristLeftReal, wristRightReal, proj1);
	glm::vec3 lerp2 = lerp(wristLeftReal, wrist3Real, proj2);
	glm::vec3 dir1 = glm::normalize(reject(lerp2 - lerp1, v2 - v1));
	glm::vec3 dir2 = glm::normalize(reject(lerp1 - lerp2, v3 - v1));
	glm::vec3 centerv3 = Intersection(lerp1, dir1, lerp2, dir2);
	std::cout << "projs:    {" << proj1 << ", " << proj2 << "}\n";
	std::cout << "lerp1:    {" << lerp1.x << ", " << lerp1.y << ", " << lerp1.z << "}\n";
	std::cout << "lerp2:    {" << lerp2.x << ", " << lerp2.y << ", " << lerp2.z << "}\n";
	std::cout << "dir1:     {" << dir1.x << ", " << dir1.y << ", " << dir1.z << "}\n";
	std::cout << "dir2:     {" << dir2.x << ", " << dir2.y << ", " << dir2.z << "}\n";
	std::cout << "centerv3: {" << centerv3.x << ", " << centerv3.y << ", " << centerv3.z << "}\n\n" << std::flush;


	glm::vec3 centerDirection = glm::normalize(centerv3 - position);
	glm::vec3 upDirection = glm::vec3(0, 1, 0);
	transform = glm::lookAt(centerv3, position, upDirection);
	rotation = glm::quatLookAt(centerDirection, upDirection);

	/* * ankle is slightly off ground in calibration, correct for it?
	*  * get orientation
	*		get projection between center and line between p1 and p2
	*		turn based on that, then turn based on rejection (or collision of all three)
	*		correct for roll
	*/

	active = true;
	// to prevent position reseting
	// https://smartglasseshub.com/disable-quest-2-proximity-sensor/
}
glm::vec3 Camera::GetVector(glm::vec2 coords) {
	coords.x -= width / 2;
	coords.y -= height / 2;
	glm::quat x = glm::quat(glm::highp_vec3(0, radPerPixel * coords.x, 0));
	glm::quat y = glm::quat(glm::highp_vec3(radPerPixel * coords.y, 0, 0));
	glm::quat final = rotation * x * y;
	return glm::rotate(final, glm::vec3(0, 0, 1)); // Confirm this vector
}