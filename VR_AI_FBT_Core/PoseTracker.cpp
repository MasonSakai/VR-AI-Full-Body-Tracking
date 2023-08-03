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
	for (uint8_t i = 0; i < 16; i++) {
		cameras[i].Init(i);
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
	glm::vec3 pos(0,0,0);
	float total = 0;

	glm::vec3 closest;
	float weight;

	for (uint8_t i = 0; i < 15; i++) {
		if (!(cameraFlags & (1 << i))) continue;
		std::cout << "pos:      {" << cameras[i].position.x << ", " << cameras[i].position.y << ", " << cameras[i].position.z << "}\n";
		std::cout << "dir:      {" << directions[i].x << ", " << directions[i].y << ", " << directions[i].z << "}\n";
		for (uint8_t j = i + 1; j < 16; j++) {
			if (!(cameraFlags & (1 << j))) continue;
			weight = scores[i] * scores[j];
			total += weight;
			closest = CalculateClosest(cameras[i].position, directions[i], cameras[j].position, directions[j]);
			pos += weight * closest;
		}
	}
	position = pos / total;
	hasValidPosition = true;

	if (tracker == Poses::right_hip) {
		if (trackers[Poses::left_hip].hasValidPosition) {
			position = (position + trackers[Poses::left_hip].position) * .5f;
		}
		else
		{
			hasValidPosition = false;
		}
	}


	/*
	Multi For: nose
	pos: {0.632671, 1.00893e-43, -0.198833}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.55663, 0.16833, 0.813528}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.632671, 0.442465, -0.635572}
		clo:  {0.559234, 1.49082, -0.0557221}, 0.400206
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.632671, 0.442465, -0.635572}
	pos:  {0.559234, 1.49082, -0.0557221}, 1
	position: {0.559234, 1.49082, -0.0557221}

	Multi For: left_eye
	pos: {0.641554, 1.00893e-43, -0.206095}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.554514, 0.184704, 0.811417}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.641554, 0.455746, -0.617012}
		clo:  {0.585305, 1.5353, -0.021517}, 0.522582
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.641554, 0.455746, -0.617012}
	pos:  {0.585305, 1.5353, -0.021517}, 1
	position: {0.585305, 1.5353, -0.021517}

	Multi For: right_eye
	pos: {0.618574, 1.00893e-43, -0.209952}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.582737, 0.233565, 0.778373}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.618574, 0.462699, -0.635041}
		clo:  {0.627535, 1.63813, -0.163791}, 0.3373
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.618574, 0.462699, -0.635041}
	pos:  {0.627535, 1.63813, -0.163791}, 1
	position: {0.627535, 1.63813, -0.163791}

	Multi For: left_ear
	pos: {0.656847, 1.00893e-43, -0.199802}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.613625, 0.233191, 0.754378}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.656847, 0.444271, -0.609241}
		clo:  {0.774297, 1.64703, -0.155695}, 0.488827
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.656847, 0.444271, -0.609241}
	pos:  {0.774297, 1.64703, -0.155695}, 1
	position: {0.774297, 1.64703, -0.155695}

	Multi For: right_ear
	pos: {0.609886, 1.00893e-43, -0.202222}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.602998, 0.233024, 0.76295}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.609886, 0.448573, -0.653316}
		clo:  {0.64384, 1.62669, -0.247312}, 0.554019
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.609886, 0.448573, -0.653316}
	pos:  {0.64384, 1.62669, -0.247312}, 1
	position: {0.64384, 1.62669, -0.247312}

	Multi For: left_shoulder
	pos: {0.699929, 1.00893e-43, -0.152654}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.670896, 0.142198, 0.72779}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.699929, 0.356515, -0.618867}
		clo:  {0.928407, 1.42375, -0.216753}, 0.763177
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.699929, 0.356515, -0.618867}
	pos:  {0.928407, 1.42375, -0.216753}, 1
	position: {0.928407, 1.42375, -0.216753}

	Multi For: right_shoulder
	pos: {0.596007, 1.00893e-43, -0.163016}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.596632, 0.184408, 0.78104}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.596007, 0.375675, -0.709679}
		clo:  {0.548202, 1.44492, -0.329912}, 0.648596
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.596007, 0.375675, -0.709679}
	pos:  {0.548202, 1.44492, -0.329912}, 1
	position: {0.548202, 1.44492, -0.329912}

	Multi For: left_elbow
	pos: {0.732598, 1.00893e-43, -0.0920184}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.67827, 0.0231271, 0.734448}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.732598, 0.239948, -0.636965}
		clo:  {0.944368, 1.10135, -0.207683}, 0.500652
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.732598, 0.239948, -0.636965}
	pos:  {0.944368, 1.10135, -0.207683}, 1
	position: {0.944368, 1.10135, -0.207683}

	Multi For: right_elbow
	pos: {0.599307, 1.00893e-43, -0.135014}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.536686, 0.0736134, 0.840565}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.599307, 0.322521, -0.732674}
		clo:  {0.397343, 1.20347, -0.183189}, 0.759468
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.599307, 0.322521, -0.732674}
	pos:  {0.397343, 1.20347, -0.183189}, 1
	position: {0.397343, 1.20347, -0.183189}

	Multi For: left_wrist
	pos: {0.761282, 1.00893e-43, -0.0384733}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.659407, -0.0661097, 0.748874}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.761282, 0.134516, -0.634315}
		clo:  {0.932284, 0.840781, -0.132294}, 0.436757
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.761282, 0.134516, -0.634315}
	pos:  {0.932284, 0.840781, -0.132294}, 1
	position: {0.932284, 0.840781, -0.132294}

	Multi For: right_wrist
	pos: {0.623623, 1.00893e-43, -0.185249}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.517202, 0.161277, 0.840531}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.623623, 0.417403, -0.66096}
		clo:  {0.44983, 1.42616, -0.0343691}, 0.509653
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.623623, 0.417403, -0.66096}
	pos:  {0.44983, 1.42616, -0.0343691}, 1
	position: {0.44983, 1.42616, -0.0343691}

	Multi For: left_hip
	pos: {0.722846, 1.00893e-43, -0.0497415}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.6442, -0.0543649, 0.762923}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.722846, 0.156923, -0.672955}
		clo:  {0.805962, 0.883483, -0.192835}, 0.706946
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.722846, 0.156923, -0.672955}
	pos:  {0.805962, 0.883483, -0.192835}, 1
	position: {0.805962, 0.883483, -0.192835}

	Multi For: right_hip
	pos: {0.656217, 1.00893e-43, -0.0502712}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.605995, -0.0523375, 0.793745}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.656217, 0.157724, -0.737904}
		clo:  {0.586123, 0.88481, -0.272954}, 0.546189
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.656217, 0.157724, -0.737904}
	pos:  {0.586123, 0.88481, -0.272954}, 1
	position: {0.586123, 0.88481, -0.272954}

	Multi For: left_knee
	pos: {0.717224, 1.00893e-43, 0.0486217}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.619363, -0.239948, 0.747539}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.717224, -0.0392327, -0.695738}
		clo:  {0.756435, 0.409879, -0.219401}, 0.573129
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.717224, -0.0392327, -0.695738}
	pos:  {0.756435, 0.409879, -0.219401}, 1
	position: {0.756435, 0.409879, -0.219401}

	Multi For: right_knee
	pos: {0.661968, 1.00893e-43, 0.0516441}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.602529, -0.240296, 0.761063}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.661968, -0.0455345, -0.748148}
		clo:  {0.610643, 0.422205, -0.30947}, 0.615638
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.661968, -0.0455345, -0.748148}
	pos:  {0.610643, 0.422205, -0.30947}, 1
	position: {0.610643, 0.422205, -0.30947}

	Multi For: left_ankle
	pos: {0.702716, 1.00893e-43, 0.139878}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.585425, -0.383888, 0.714078}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.702716, -0.220798, -0.676342}
		clo:  {0.753278, -0.0202498, -0.210695}, 0.558246
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.702716, -0.220798, -0.676342}
	pos:  {0.753278, -0.0202498, -0.210695}, 1
	position: {0.753278, -0.0202498, -0.210695}

	Multi For: right_ankle
	pos: {0.634169, 1.00893e-43, 0.139936}
	pos:      {-0.877877, 1.01514, -2.18955}
	dir:      {0.573922, -0.412011, 0.707715}
		pos:  {-0.889765, 0.52607, 1.38652}
		dir:  {0.634169, -0.221266, -0.740858}
		clo:  {0.603167, -0.0194275, -0.373586}, 0.641282
	pos:      {-0.889765, 0.52607, 1.38652}
	dir:      {0.634169, -0.221266, -0.740858}
	pos:  {0.603167, -0.0194275, -0.373586}, 1
	position: {0.603167, -0.0194275, -0.373586}
	*/
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

	uint8_t n = 0;
	for (n = 0; n < 17; n++)
		if (cameraFlags & (1 << n))
			break;
	if (n == 16) return false;
	float dist;

	hasValidPosition = true;

	switch (tracker)
	{
	case Poses::nose:
	case Poses::left_eye:
	case Poses::right_eye:
	case Poses::left_ear:
	case Poses::right_ear:
		break;
	case Poses::right_hip:
		if (trackers[Poses::left_hip].hasValidPosition) {
			dist = glm::length(cameras[n].position - trackers[Poses::left_hip].position);
			position = cameras[n].position + directions[n] * dist;

			position = (position + trackers[Poses::left_hip].position) * .5f;
		}
		else
		{
			hasValidPosition = false;
		}
		break;
	case Poses::left_wrist:
		position = leftHandPosReal + leftHandRotReal * handToWrist;
		break;
	case Poses::right_wrist:
		position = rightHandPosReal + rightHandRotReal * handToWrist;
		break;
	default:
		dist = glm::length(cameras[n].position - headPosReal);
		position = cameras[n].position + directions[n] * dist;
		break;
	}
	return hasValidPosition;
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
		rotation = glm::quatLookAt(glm::normalize(trackers[Poses::left_hip].position - position),
								   glm::normalize(trackers[Poses::right_shoulder].position - position));
		break;

		//use floor when on or below
		//put toes on when near (<60/70 deg)
		//use leg when above
	case Poses::left_ankle:
		rotation = glm::quatLookAt(reject(trackers[right_hip].rotation * glm::vec3(1, 0, 0),
			glm::vec3(0, 1, 0)), glm::vec3(0, 1, 0));
		break;
	case Poses::right_ankle:
		rotation = glm::quatLookAt(reject(trackers[right_hip].rotation * glm::vec3(1, 0, 0),
			glm::vec3(0, 1, 0)), glm::vec3(0, 1, 0));
		break;

	default:
		//std::cout << "Defaulted" << (int)tracker << "\n";
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

void Camera::Init(uint8_t index) {
	this->index = index;
	active = false;
	connected = false;
	waitingForSize = true;
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
	//std::cout << "v1: {" << v1.x << ", " << v1.y << ", " << v1.z << "}\n";
	//std::cout << "v2: {" << v2.x << ", " << v2.y << ", " << v2.z << "}\n";
	//std::cout << "v3: {" << v3.x << ", " << v3.y << ", " << v3.z << "}\n\n" << std::flush;

	glm::vec3 planeNorm = glm::normalize(glm::cross(v3 - v1, v2 - v1));

	//std::cout << "pos: {" << position.x << ", " << position.y << ", " << position.z << "}\n";
	//std::cout << "norm: {" << planeNorm.x << ", " << planeNorm.y << ", " << planeNorm.z << "}\n\n" << std::flush;

	glm::vec3 positions[17];
	glm::vec3 pos, dir;

	for (int i = 0; i < 17; i++) {
		dir = GetVector(trackers[i].GetPose(index));
		positions[i] = IntersectPlane(v1, planeNorm, position, dir);
		trackers[i].position = positions[i];
		trackers[i].rotation = rotation;
		//std::cout << i << ": {" << positions[i].x << ", " << positions[i].y << ", " << positions[i].z << "}\n";
		//std::cout << "dir: {" << dir.x << ", " << dir.y << ", " << dir.z << "}\n\n";
	}

	float ats = (trackers[Poses::left_ankle].position.y + trackers[Poses::right_ankle].position.y) * .5f;
	ankleToSole = (ankleToSole * distancesRecorded + ats) / (distancesRecorded + 1);
	float sts = glm::length(trackers[Poses::left_shoulder].position - trackers[Poses::right_shoulder].position);
	shoulderToShoulder = (shoulderToShoulder * distancesRecorded + sts) / (distancesRecorded + 1);
	float hw = glm::length(trackers[Poses::left_hip].position - trackers[Poses::right_hip].position);
	hipWidth = (hipWidth * distancesRecorded + hw) / (distancesRecorded + 1);
	float sth = (glm::length(trackers[Poses::left_hip].position - trackers[Poses::left_shoulder].position) + 
		glm::length(trackers[Poses::right_hip].position - trackers[Poses::right_shoulder].position)) * .5f;
	shoulderToHip = (shoulderToHip * distancesRecorded + sth) / (distancesRecorded + 1);
	float ua = (glm::length(trackers[Poses::left_elbow].position - trackers[Poses::left_shoulder].position) +
		glm::length(trackers[Poses::right_elbow].position - trackers[Poses::right_shoulder].position)) * .5f;
	upperArmLen = (upperArmLen * distancesRecorded + ua) / (distancesRecorded + 1);
	float la = (glm::length(trackers[Poses::left_elbow].position - trackers[Poses::left_wrist].position) +
		glm::length(trackers[Poses::right_elbow].position - trackers[Poses::right_wrist].position)) * .5f;
	lowerArmLen = (lowerArmLen * distancesRecorded + la) / (distancesRecorded + 1);
	float ul = (glm::length(trackers[Poses::left_hip].position - trackers[Poses::left_knee].position) +
		glm::length(trackers[Poses::right_hip].position - trackers[Poses::right_knee].position)) * .5f;
	upperLegLen = (upperLegLen * distancesRecorded + ul) / (distancesRecorded + 1);
	float ll = (glm::length(trackers[Poses::left_ankle].position - trackers[Poses::left_knee].position) +
		glm::length(trackers[Poses::right_ankle].position - trackers[Poses::right_knee].position)) * .5f;
	lowerLegLen = (lowerLegLen * distancesRecorded + ll) / (distancesRecorded + 1);
	distancesRecorded++;

	std::cout << "ankleToSole        " << ankleToSole << std::endl;
	std::cout << "shoulderToShoulder " << shoulderToShoulder << std::endl;
	std::cout << "hipWidth           " << hipWidth << std::endl;
	std::cout << "shoulderToHip      " << shoulderToHip << std::endl;
	std::cout << "upperArmLen        " << upperArmLen << std::endl;
	std::cout << "lowerArmLen        " << lowerArmLen << std::endl;
	std::cout << "upperLegLen        " << upperLegLen << std::endl;
	std::cout << "lowerLegLen        " << lowerLegLen << std::endl << std::endl;
}