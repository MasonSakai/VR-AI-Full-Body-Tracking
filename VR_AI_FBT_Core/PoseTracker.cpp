#include "PoseTracker.h"

const float handToWristRatio = .88;

float htwRecorded = 0;
glm::vec3 handToWrist;
float distancesRecorded = 0;
float shoulderToShoulder,
shoulderToHip, hipWidth,
upperArmLen, lowerArmLen,
upperLegLen, lowerLegLen,
ankleToSole;

Camera cameras[16];
PoseTracker trackers[17];

void PoseTracker::InitTrackers() {
	for (uint8_t i = 0; i < 17; i++) {
		trackers[i].Init(i);
	}
}

void PoseTracker::SetPose(uint8_t camera, QJsonObject poseData) {
	QJsonObject json;
	for (int i = 0; i < 17; i++) {
		json = poseData[PoseNames[i].c_str()].toObject();
		trackers[i].UpdatePose(camera, json["x"].toDouble(), json["y"].toDouble(), json["score"].toDouble());
	}
}

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

	//if this doesn't work well, take n, put p1 and p2 onto same plane, find intersection with util, then use against plane offset to find center
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

	glm::vec3 closest;

	std::cout << "Multi For: " << PoseNames[tracker] << "\n";

	for (uint8_t i = 0; i < 15; i++) {
		if (!(cameraFlags & (1 << i))) continue;
		std::cout << "pos:      {" << cameras[i].position.x << ", " << cameras[i].position.y << ", " << cameras[i].position.z << "}\n";
		std::cout << "dir:      {" << directions[i].x << ", " << directions[i].y << ", " << directions[i].z << "}\n";
		for (uint8_t j = i + 1; j < 16; j++) {
			if (!(cameraFlags & (1 << j))) continue;
			float weight = scores[i] * scores[j];
			total += weight;
			closest = CalculateClosest(cameras[i].position, directions[i], cameras[j].position, directions[j]);
			pos += weight * closest;
			std::cout << "    pos:  {" << cameras[j].position.x << ", " << cameras[j].position.y << ", " << cameras[j].position.z << "}\n";
			std::cout << "    dir:  {" << directions[j].x << ", " << directions[j].y << ", " << directions[j].z << "}\n";
			std::cout << "    clo:  {" << closest.x << ", " << closest.y << ", " << closest.z << "}, " << weight << "\n";
		}
	}
	position = pos / total;
	std::cout << "pos:  {" << pos.x << ", " << pos.y << ", " << pos.z << "}, " << total << "\n";
	std::cout << "position: {" << position.x << ", " << position.y << ", " << position.z << "}\n\n" << std::flush;
	hasValidPosition = true;
}

uint8_t PoseTracker::CalculatePosition() {
	hasValidPosition = false;
	uint8_t n = getNumberOfCams();
	if (n == 0) return 0;
	UpdateDirections();
	if (n == 1) return 1;
	CalculateMultiPosition();
	return 2;
}

//return true if able to make pose
bool PoseTracker::CalculateSingleCameraPosition() {
	if (tracker == Poses::left_wrist) {
		position = leftHandPosReal + leftHandRotReal * handToWrist;
		hasValidPosition = true;
		return true;
	}
	if (tracker == Poses::right_wrist) {
		position = rightHandPosReal + rightHandRotReal * handToWrist;
		hasValidPosition = true;
		return true;
	}

	uint8_t n = 0;
	for (n = 0; n < 17; n++)
		if (cameraFlags & (1 << n))
			break;
	if (n == 16) return false;

	float dist = glm::length(cameras[n].position - headPosReal);

	position = cameras[n].position + directions[n] * dist;

	hasValidPosition = true;
	return true;
}

uint8_t PoseTracker::CalculateOrientation() {
	switch (tracker) {
		//mimic real
	case Poses::left_wrist:
		rotation = leftHandRotReal;
		break;
	case Poses::right_wrist:
		rotation = rightHandRotReal;
		break;

	case Poses::right_hip:
		rotation = glm::quatLookAt(trackers[Poses::left_hip].position - position, trackers[Poses::right_shoulder].position - position);
		break;

		//use floor when on or below
		//put toes on when near (<60/70 deg)
		//use leg when above
	case Poses::left_ankle:
		rotation = leftHandRotReal;
		break;
	case Poses::right_ankle:
		rotation = rightHandRotReal;
		break;

	default:
		std::cout << "Defaulted" << (int)tracker << "\n";
		return 0;
	}
	return 1;
}

void Camera::SetSize(uint8_t index, uint16_t width, uint16_t height) {
	cameras[index].width = width;
	cameras[index].height = height;
	cameras[index].waitingForSize = false;
}
void Camera::OnConnect(uint8_t index) {
	VRDashboardOverlay::SharedInstance()->SetCameraState(index, 5);
	cameras[index].active = false;
	cameras[index].connected = true;
	CreateCameraOverlay(index);
}
void Camera::OnStart(uint8_t index) {
	VRDashboardOverlay::SharedInstance()->SetCameraState(index, 4);
}

// _1 (left) and _2 (right) are from T-pose
// _3 is next to foot
// position is against the camera
// qp is against camera, q3 is against ground, used to correct the camera's position
void Camera::Calibrate(glm::vec3 position, glm::quat qp,
	glm::vec3 v1, glm::vec2 p1, glm::quat q1,
	glm::vec3 v2, glm::vec2 p2, glm::quat q2,
	glm::vec3 v3, glm::vec2 p3, glm::quat q3) {

	/*std::cout << "pos: {" << position.x << ", " << position.y << ", " << position.z << "}\n";
	std::cout << "v1: {" << v1.x << ", " << v1.y << ", " << v1.z << "}\n";
	std::cout << "p1: {" << p1.x << ", " << p1.y << "}\n";
	std::cout << "v2: {" << v2.x << ", " << v2.y << ", " << v2.z << "}\n";
	std::cout << "p2: {" << p2.x << ", " << p2.y << "}\n";
	std::cout << "v3: {" << v3.x << ", " << v3.y << ", " << v3.z << "}\n";
	std::cout << "p3: {" << p3.x << ", " << p3.y << "}\n\n" << std::flush;*/

	//correct camera position
	glm::vec3 cameraPosOffset = glm::rotate(glm::inverse(q3), glm::vec3(0, -v3.y, 0));
	position += glm::rotate(qp, cameraPosOffset);
	this->position = position;
	/*std::cout << "cameraPosOffset: {" << cameraPosOffset.x << ", " << cameraPosOffset.y << ", " << cameraPosOffset.z << "}\n";
	std::cout << "pos: {" << position.x << ", " << position.y << ", " << position.z << "}\n\n" << std::flush;*/

	//Get wrist real positions
	glm::vec3 handToHandDelta = v2 - v1;
	glm::vec3 handToHandCenter = (v1 + v2) / 2.0f;
	glm::vec3 wristLeftReal = handToHandCenter - handToHandDelta * handToWristRatio * .5f;
	glm::vec3 wristRightReal = handToHandCenter + handToHandDelta * handToWristRatio * .5f;

	//correct wrist position with hand to wrist ratio
	glm::vec3 handToWristLeft = wristLeftReal - v1;
	glm::vec3 handToWristRight = wristRightReal - v2;
	/*std::cout << "handToWristLeft:  {" << handToWristLeft.x << ", " << handToWristLeft.y << ", " << handToWristLeft.z << "}\n";
	std::cout << "handToWristRight: {" << handToWristRight.x << ", " << handToWristRight.y << ", " << handToWristRight.z << "}\n";*/
	//get wrist offsets using q1 and q2
	handToWristLeft = glm::rotate(glm::inverse(q1), handToWristLeft);
	handToWristRight = glm::rotate(glm::inverse(q2), handToWristRight);
	handToWrist = (handToWrist * htwRecorded + (handToWristLeft + handToWristRight) / 2.0f) / (htwRecorded + 1);
	htwRecorded++;
	/*std::cout << "handToWristLeft:  {" << handToWristLeft.x << ", " << handToWristLeft.y << ", " << handToWristLeft.z << "}\n";
	std::cout << "handToWristRight: {" << handToWristRight.x << ", " << handToWristRight.y << ", " << handToWristRight.z << "}\n";
	std::cout << "handToWrist:      {" << handToWrist.x << ", " << handToWrist.y << ", " << handToWrist.z << "}\n\n" << std::flush;*/

	//get wrist3 position
	glm::vec3 wrist3Real = v3 + glm::rotate(q3, handToWrist);
	/*std::cout << "v3:         {" << v3.x << ", " << v3.y << ", " << v3.z << "}\n";
	std::cout << "wrist3Real: {" << wrist3Real.x << ", " << wrist3Real.y << ", " << wrist3Real.z << "}\n\n" << std::flush;*/

	//get radPerPixel
	glm::vec3 wLRDel = wristLeftReal - position;
	glm::vec3 wRRDel = wristRightReal - position;
	float wtwRad = acosf(glm::dot(wLRDel, wRRDel) / (glm::length(wLRDel) * glm::length(wRRDel)));
	float wtwPix = glm::length(p2 - p1);
	radPerPixel = wtwRad / wtwPix;
	//std::cout << "radPerPixel: " << radPerPixel << "\n\n" << std::flush;

	//get center in 3d space
	glm::vec2 center = glm::vec2(width, height) * .5f;
	float proj1 = glm::dot(center - p1, p2 - p1) / glm::length2(p2 - p1);
	float proj2 = glm::dot(center - p1, p3 - p1) / glm::length2(p3 - p1);
	glm::vec3 lerp1 = lerp(wristLeftReal, wristRightReal, proj1);
	glm::vec3 lerp2 = lerp(wristLeftReal, wrist3Real, proj2);
	glm::vec3 norm = glm::cross(wristRightReal - wristLeftReal, wrist3Real - wristLeftReal);
	glm::vec3 dir1 = glm::normalize(reject(lerp2 - lerp1, v2 - v1));
	glm::vec3 dir2 = glm::normalize(reject(lerp2 - lerp1, v3 - v1));
	dir1 = reject(dir1, norm);
	dir2 = reject(dir2, norm);
	glm::vec3 centerv3 = Intersection(lerp1, dir1, lerp2, dir2);
	glm::vec3 centerDirection = glm::normalize(centerv3 - position);
	/*std::cout << "projs:    {" << proj1 << ", " << proj2 << "}\n";
	std::cout << "lerp1:    {" << lerp1.x << ", " << lerp1.y << ", " << lerp1.z << "}\n";
	std::cout << "lerp2:    {" << lerp2.x << ", " << lerp2.y << ", " << lerp2.z << "}\n";
	std::cout << "dir1:     {" << dir1.x << ", " << dir1.y << ", " << dir1.z << "}\n";
	std::cout << "dir2:     {" << dir2.x << ", " << dir2.y << ", " << dir2.z << "}\n";
	std::cout << "centerv3: {" << centerv3.x << ", " << centerv3.y << ", " << centerv3.z << "}\n\n" << std::flush;*/

	//Correct for roll
	glm::vec2 pd = p2 - p1;
	float angle = atan2f(pd.y, pd.x);
	glm::vec3 upDirection = glm::rotate(glm::angleAxis(angle, centerDirection), glm::normalize(glm::cross(v1 - position, v2 - position)));

	transform = glm::lookAt(centerv3, position, upDirection);
	rotation = glm::quatLookAt(-centerDirection, upDirection);

	active = true;

	// to prevent position reseting
	// https://smartglasseshub.com/disable-quest-2-proximity-sensor/
}
glm::vec3 Camera::GetVector(glm::vec2 coords) {
	//Double check this math
	coords.x -= width / 2;
	coords.y -= height / 2;
	glm::quat x = glm::quat(glm::vec3(0, radPerPixel * coords.x, 0));
	glm::quat y = glm::quat(glm::vec3(radPerPixel * coords.y, 0, 0));
	glm::quat final = rotation * x * y;
	return glm::rotate(final, glm::vec3(0, 0, 1)); // Confirm this vector
}

void Camera::CalibrateDistances(glm::vec3 v1, glm::quat q1, glm::vec3 v2, glm::quat q2, glm::vec3 v3) {
	std::cout << "v1: {" << v1.x << ", " << v1.y << ", " << v1.z << "}\n";
	std::cout << "v2: {" << v2.x << ", " << v2.y << ", " << v2.z << "}\n";
	std::cout << "v3: {" << v3.x << ", " << v3.y << ", " << v3.z << "}\n\n" << std::flush;

	glm::vec3 norm = glm::vec3(transform[0][2], transform[1][2], transform[2][2]);
	std::cout << "norm: {" << norm.x << ", " << norm.y << ", " << norm.z << "}\n";
	std::cout << "len: " << glm::length(norm) << "\n\n" << std::flush;

	trackers[Poses::right_hip].position = position + norm * 1.0f;
	trackers[Poses::left_ankle].position = position;

	//float sts;
}

/*
pos: {-0.42421, 1.01416, -2.32494}
v1: {-0.345276, 1.29682, 0.706046}
p1: {87.077, 85.8325}
v2: {1.23155, 1.31366, -0.133572}
p2: {379.711, 69.6272}
v3: {0.701194, -0.0242015, 0.243199}
p3: {297.903, 328.608}

cameraPosOffset: {0.00458194, -0.00167493, 0.0237047}
pos: {-0.413826, 1.00995, -2.30349}

projs:    {0.764435, 0.836989}
lerp1:    {0.810067, 1.30916, 0.0908552}
lerp2:    {0.507529, 0.264109, 0.274884}
dir1:     {0.017451, -0.999767, 0.012721}
dir2:     {-0.661834, -0.653586, 0.366642}
centerv3: {0.192185, -0.047305, 0.449578}

v1: {-0.345276, 1.29682, 0.706046}
v2: {1.23155, 1.31366, -0.133572}
v3: {0.701194, -0.0242015, 0.243199}

norm: {0.201284, -0.351162, 0.914423}
len: 1
*/