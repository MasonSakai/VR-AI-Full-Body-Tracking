#include "PoseTracker.h"

const float handToWristRatio = .88,
			footToLowerLegRatio = .16666667f;

float htwRecorded = 0;
glm::vec3 handToWrist;
float distancesRecorded = 0;
float shoulderToShoulder,
shoulderToHip, hipWidth,
upperArmLen, lowerArmLen,
upperLegLen, lowerLegLen,
ankleToSole, footLen, maxAnkle;

Camera cameras[16];
PoseTracker trackers[17];

bool PoseTrackers[17] = {
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

float trackerDampening = .5f;

void PoseTracker::InitTrackers() {
	QJsonObject trackerConfig = config["trackers"].toObject();
	PoseTrackers[Poses::left_ankle] = trackerConfig["ankle"].toBool();
	PoseTrackers[Poses::right_ankle] = PoseTrackers[Poses::left_ankle];
	PoseTrackers[Poses::left_knee] = trackerConfig["knee"].toBool();
	PoseTrackers[Poses::right_knee] = PoseTrackers[Poses::left_knee];
	PoseTrackers[Poses::right_hip] = trackerConfig["hip"].toBool();
	PoseTrackers[Poses::left_shoulder] = trackerConfig["shoulder"].toBool();
	PoseTrackers[Poses::right_shoulder] = PoseTrackers[Poses::left_shoulder];
	PoseTrackers[Poses::left_elbow] = trackerConfig["elbow"].toBool();
	PoseTrackers[Poses::right_elbow] = PoseTrackers[Poses::left_elbow];
	trackerDampening = config["trackerDampening"].toDouble(.5);

	for (uint8_t i = 0; i < 17; i++) {
		trackers[i].Init(i);
	}
	for (uint8_t i = 0; i < 16; i++) {
		cameras[i].Init(i);
	}

	GetTrackers();
}

void PoseTracker::Exit() {
	QJsonObject configObj = config.object();
	configObj.insert("trackerDampening", trackerDampening);
	QJsonObject trackerConfig = configObj["trackers"].toObject();
	trackerConfig.insert("ankle", PoseTrackers[Poses::left_ankle]);
	trackerConfig.insert("knee", PoseTrackers[Poses::left_knee]);
	trackerConfig.insert("hip", PoseTrackers[Poses::right_hip]);
	trackerConfig.insert("shoulder", PoseTrackers[Poses::left_shoulder]);
	trackerConfig.insert("elbow", PoseTrackers[Poses::left_elbow]);
	configObj.insert("trackers", trackerConfig);
	config.setObject(configObj);

	for (int i = 0; i < 17; i++) {
		if (PoseTrackers[i]) {
			deleteVirtualDevice(trackerIDs[i]);
		}
	}
}

void PoseTracker::SetPose(uint8_t camera, QJsonObject poseData) {
	QJsonObject json;
	for (int i = 0; i < 17; i++) {
		if (poseData[PoseNames[i].c_str()].isObject()) {
			json = poseData[PoseNames[i].c_str()].toObject();
			trackers[i].UpdatePose(camera, json["x"].toDouble(), json["y"].toDouble(), json["score"].toDouble());
		}
		else {
			trackers[i].ClearPose(camera);
		}
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

	return (c1 + c2) * .5f;

	//have it give distance?
}

void PoseTracker::UpdateDirections() {
	for (int i = 0; i < 16; i++) {
		if (cameraFlags & (1 << i)) {
			directions[i] = cameras[i].GetVector(points[i]);
		}
	}
}

void PoseTracker::UpdatePositionDampened(glm::vec3 pos) {
	position = trackerDampening * position + (1 - trackerDampening) * pos;
}

void PoseTracker::CalculateMultiPosition() {
	glm::vec3 pos(0,0,0);
	float total = 0;

	glm::vec3 closest;
	float weight;

	for (uint8_t i = 0; i < 15; i++) {
		if (!(cameraFlags & (1 << i))) continue;
		for (uint8_t j = i + 1; j < 16; j++) {
			if (!(cameraFlags & (1 << j))) continue;
			weight = scores[i] * scores[j];
			total += weight;
			//disregard if too far/out of range?
			closest = CalculateClosest(cameras[i].position, directions[i], cameras[j].position, directions[j]);
			pos += weight * closest;
		}
	}
	UpdatePositionDampened(pos / total);
	hasValidPosition = true;

	if (tracker == Poses::right_hip) {
		hipPosValid = true;
		hipRightRealPos = position;
		if (trackers[Poses::left_hip].hasValidPosition) {
			position = (hipRightRealPos + trackers[Poses::left_hip].position) * .5f;
		}
		else
		{
			hasValidPosition = false;
		}
	}
}

uint8_t PoseTracker::CalculatePosition() {
	hasValidPosition = false;
	hipPosValid = false;
	hasAmbiguousPosition = false;
	hasDualPosition = false;

	uint8_t n = getNumberOfCams();
	if (n == 0) return 0;
	UpdateDirections();
	if (n == 1) return 1;
	CalculateMultiPosition();
	return 2;
}

uint8_t PoseTracker::CalculateOneValid(glm::vec3 center, float r, glm::vec3 pos, glm::vec3 dir) {
	glm::vec3 p1, p2;
	uint8_t count = IntersectSphere(center, r, pos, dir, &p1, &p2, 0.025f);
	switch (count)
	{
	case 0:
		hasAmbiguousPosition = false;
		hasDualPosition = false;
		amb1 = p1;
		amb2 = glm::normalize(p1 - center) * r + center;
		break;
	case 1:
		hasAmbiguousPosition = false;
		hasDualPosition = false;
		amb1 = p1;
	case 2:
		hasAmbiguousPosition = true;
		hasDualPosition = true;
		amb1 = p1;
		amb2 = p2;
		break;
	}
	return count;
}
uint8_t PoseTracker::CalculateTwoValid(glm::vec3 p1, float r1, glm::vec3 p2, float r2, glm::vec3 pos, glm::vec3 dir) {
	float d = glm::length(p2 - p1);
	float d1 = (r2 * r2 - r1 * r1 - d * d) / (-2 * d);
	float r = sqrtf(r1 * r1 - d1 * d1);
	glm::vec3 center = lerp(p1, p2, d1 / d);
	glm::vec3 norm = glm::normalize(p2 - p1);
	if (fabsf(glm::dot(norm, dir)) < 0.1f) {
		return CalculateOneValid(center, r, pos, dir);
	}
	else {
		glm::vec3 p = IntersectPlane(center, norm, pos, dir);
		float rad = glm::length(pos - center);
		if (fabsf(r - rad) < .05f) {
			hasAmbiguousPosition = false;
			amb1 = p;
			return 1;
		}
		else {
			//if this returns false there's a serious issue with something...
			hasAmbiguousPosition = true;
			amb1 = p;
			amb2 = center + glm::normalize(p - center) * r;
			return 0;
		}
	}
}


//return true if able to make pose
bool PoseTracker::CalculateSingleCameraPosition() {
	if (hasValidPosition) return true;

	uint8_t n = 0;
	for (n = 0; n < 17; n++)
		if (cameraFlags & (1 << n))
			break;
	if (n == 16) return false;
	float dist;

	hasValidPosition = false;
	hasAmbiguousPosition = false;
	hasDualPosition = false;

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
			if (!hipPosValid) {
				dist = glm::length(cameras[n].position - trackers[Poses::left_hip].position);
				hipRightRealPos = cameras[n].position + directions[n] * dist;
			}

			UpdatePositionDampened((hipRightRealPos + trackers[Poses::left_hip].position) * .5f);
			hasValidPosition = true;
		}
		break;
	case Poses::left_wrist:
		UpdatePositionDampened(leftHandPosReal + leftHandRotReal * handToWrist);
		hasValidPosition = true;
		break;
	case Poses::right_wrist:
		UpdatePositionDampened(rightHandPosReal + rightHandRotReal * handToWrist);
		hasValidPosition = true;
		break;

	case Poses::left_ankle:
		CalculateSingleAnkle(n, Poses::left_knee, Poses::left_hip);
		break;
	case Poses::right_ankle:
		CalculateSingleAnkle(n, Poses::right_knee, Poses::right_hip);
		break;

	case Poses::left_knee:
		CalculateSingleKnee(n, Poses::left_ankle, Poses::left_hip);
		break;
	case Poses::right_knee:
		CalculateSingleKnee(n, Poses::right_ankle, Poses::right_hip);
		break;

	default:
		dist = glm::length(cameras[n].position - headPosReal);
		UpdatePositionDampened(cameras[n].position + directions[n] * dist);
		hasValidPosition = true;
		break;
	}
	return hasValidPosition;
}

void PoseTracker::CalculateSingleAnkle(uint8_t n, uint8_t knee, uint8_t hip) {
	if (trackers[knee].hasValidPosition) {
		uint8_t c = CalculateOneValid(trackers[knee].position, lowerLegLen, cameras[n].position, directions[n]);
		if (c == 2) {
			if (trackers[Poses::left_hip].hasValidPosition && trackers[Poses::right_hip].hasValidPosition) {
				glm::vec3 c1 = glm::cross(amb1 - trackers[knee].position,
					trackers[hip].position - trackers[knee].position);
				glm::vec3 c2 = glm::cross(amb2 - trackers[knee].position,
					trackers[hip].position - trackers[knee].position);
				bool p1v = glm::dot(c1, trackers[Poses::right_hip].position - trackers[Poses::left_hip].position) > 0;
				bool p2v = glm::dot(c2, trackers[Poses::right_hip].position - trackers[Poses::left_hip].position) > 0;
				if (p1v == p2v) {
					float d1 = glm::length(amb1 - position);
					float d2 = glm::length(amb2 - position);
					if (d1 < d2)
						UpdatePositionDampened(amb1);
					else
						UpdatePositionDampened(amb2);
				}
				else if (p1v) {
					UpdatePositionDampened(amb1);
				}
				else if (p2v) {
					UpdatePositionDampened(amb2);
				}
				hasValidPosition = true;
			}
			else {
				hasValidPosition = false;
				//
			}
		}
		else if (c == 1) {
			UpdatePositionDampened(amb1);
			hasValidPosition = true;
		}
		else if (c == 0) {
			UpdatePositionDampened((position + amb2) * .5f);
			hasValidPosition = true;
			hasAmbiguousPosition = true;
		}
	}
	else if (trackers[knee].hasAmbiguousPosition && trackers[knee].hasDualPosition) {
		glm::vec3 p1, p2, p3, p4, c;
		bool b1 = false, b2 = false, b3 = false, b4 = false;;
		uint8_t n = 0;
		uint8_t n1 = CalculateOneValid(trackers[knee].amb1, lowerLegLen, cameras[n].position, directions[n]);
		p1 = amb1; p2 = amb2;
		uint8_t n2 = CalculateOneValid(trackers[knee].amb2, lowerLegLen, cameras[n].position, directions[n]);
		p3 = amb1; p4 = amb2;
		if (n1 > 0) {
			c = glm::cross(p1 - trackers[knee].position,
				trackers[hip].position - trackers[knee].position);
			b1 = glm::dot(c, trackers[Poses::right_hip].position - trackers[hip].position) > 0;
			n++;
		}
		if (n1 == 2) {
			c = glm::cross(p2 - trackers[knee].position,
				trackers[hip].position - trackers[knee].position);
			b2 = glm::dot(c, trackers[Poses::right_hip].position - trackers[hip].position) > 0;
			n++;
		}
		if (n2 > 0) {
			c = glm::cross(p3 - trackers[knee].position,
				trackers[hip].position - trackers[knee].position);
			b3 = glm::dot(c, trackers[Poses::right_hip].position - trackers[hip].position) > 0;
			n++;
		}
		if (n2 == 2) {
			c = glm::cross(p4 - trackers[knee].position,
				trackers[hip].position - trackers[knee].position);
			b4 = glm::dot(c, trackers[Poses::right_hip].position - trackers[hip].position) > 0;
			n++;
		}
		switch (n) {
		case 0:
			hasValidPosition = false;
			hasAmbiguousPosition = false;
			hasDualPosition = false;
			break;
		case 1:
			hasValidPosition = true;
			if (b1) UpdatePositionDampened(p1);
			else if (b2) UpdatePositionDampened(p2);
			else if (b3) UpdatePositionDampened(p3);
			else if (b4) UpdatePositionDampened(p4);
			else hasValidPosition = false;
			break;
		case 2:
			hasValidPosition = false;
			if (b1) amb1 = p1;
			else if (b2) amb1 = p2;
			else if (b3) amb1 = p3;
			if (b2) amb2 = p2;
			else if (b3) amb2 = p3;
			else if (b4) amb2 = p4;

			if (b1 && b2) {
				trackers[knee].hasValidPosition = true;
				trackers[knee].UpdatePositionDampened(trackers[knee].amb1);
			}
			else if (b3 && b4) {
				trackers[knee].hasValidPosition = true;
				trackers[knee].UpdatePositionDampened(trackers[knee].amb2);
			}
			hasAmbiguousPosition = true;
			hasDualPosition = true;
			break;
		case 3:
		case 4:
			float f1 = INFINITY, f2 = INFINITY, f3 = INFINITY, f4 = INFINITY, sf, nsf;
			int sn, nsn;
			if (b1) f1 = glm::length(p1 - position);
			if (b2) f2 = glm::length(p2 - position);
			if (b3) f3 = glm::length(p3 - position);
			if (b4) f4 = glm::length(p4 - position);
			sf = f1; sn = 1;
			if (f2 < sf) {
				nsf = sf; sf = f2;
				nsn = sn; sn = 2;
			}
			if (f3 < sf) {
				nsf = sf; sf = f3;
				nsn = sn; sn = 3;
			}
			if (f4 < sf) {
				nsf = sf; sf = f4;
				nsn = sn; sn = 4;
			}
			if (sn == 1) amb1 = p1;
			else if (sn == 2) amb1 = p2;
			else if (sn == 3) amb1 = p3;
			else amb1 = p4;
			if (nsn == 1) amb2 = p1;
			else if (nsn == 2) amb2 = p2;
			else if (nsn == 3) amb2 = p3;
			else amb2 = p4;
			hasAmbiguousPosition = true;
			break;
		}
	}
}
void PoseTracker::CalculateSingleKnee(uint8_t n, uint8_t ankle, uint8_t hip) {
	if (trackers[hip].hasValidPosition && trackers[ankle].hasValidPosition) {
		uint8_t c = CalculateTwoValid(trackers[hip].position, upperLegLen,
			trackers[ankle].position, lowerLegLen,
			cameras[n].position, directions[n]);
		switch (c) {
		case 0:
			UpdatePositionDampened((position + amb2) * .5f);
			hasValidPosition = true;
			break;
		case 1:
			UpdatePositionDampened(amb1);
			hasValidPosition = true;
			break;
		case 2:
			if (trackers[Poses::left_hip].hasValidPosition && trackers[Poses::right_hip].hasValidPosition) {
				glm::vec3 c1 = glm::cross(trackers[ankle].position - amb1,
					trackers[hip].position - amb1);
				glm::vec3 c2 = glm::cross(trackers[ankle].position - amb2,
					trackers[hip].position - amb2);
				bool p1v = glm::dot(c1, trackers[Poses::right_hip].position - trackers[Poses::left_hip].position) > 0;
				bool p2v = glm::dot(c2, trackers[Poses::right_hip].position - trackers[Poses::left_hip].position) > 0;
				if (p1v == p2v) {
					float d1 = glm::length(amb1 - position);
					float d2 = glm::length(amb2 - position);
					if (d1 < d2)
						UpdatePositionDampened(amb1);
					else
						UpdatePositionDampened(amb2);
				}
				else if (p1v) {
					UpdatePositionDampened(amb1);
				}
				else if (p2v) {
					UpdatePositionDampened(amb2);
				}
				hasValidPosition = true;
			}
			else {
				//Needs better check?
				if (glm::length2(position - amb1) < glm::length2(position - amb2))
					UpdatePositionDampened(amb1);
				else
					UpdatePositionDampened(amb2);
				hasValidPosition = true;
			}
			break;
		}
	}
	else if (trackers[hip].hasValidPosition) {
		uint8_t c = CalculateOneValid(trackers[hip].position, lowerLegLen, cameras[n].position, directions[n]);
		switch (c) {
		case 0:
			UpdatePositionDampened((position + amb2) * .5f);
			hasValidPosition = true;
			break;
		case 1:
			UpdatePositionDampened(amb1);
			hasValidPosition = true;
			break;
		case 2:
			/*//Needs better check?
			//for the moment keeping ambiguous since the ankles should fix it's position
			if (glm::length2(position - amb1) < glm::length2(position - amb2))
				UpdatePositionDampened(amb1;
			else
				UpdatePositionDampened(amb2;
			hasValidPosition = true;*/
			break;
		}
	}
	else if (trackers[ankle].hasValidPosition) {
		uint8_t c = CalculateOneValid(trackers[ankle].position, lowerLegLen, cameras[n].position, directions[n]);
		switch (c) {
		case 0:
			UpdatePositionDampened((position + amb2) * .5f);
			hasValidPosition = true;
			break;
		case 1:
			UpdatePositionDampened(amb1);
			hasValidPosition = true;
			break;
		case 2:
			//Needs better check?
			if (glm::length2(position - amb1) < glm::length2(position - amb2))
				UpdatePositionDampened(amb1);
			else
				UpdatePositionDampened(amb2);
			hasValidPosition = true;
			break;
		}
	}
}
void PoseTracker::CalculateSingleElbow(uint8_t n, uint8_t wrist, uint8_t shoulder) {
	if (trackers[shoulder].hasValidPosition && trackers[wrist].hasValidPosition) {
		uint8_t c = CalculateTwoValid(trackers[shoulder].position, upperLegLen,
			trackers[wrist].position, lowerLegLen,
			cameras[n].position, directions[n]);
		switch (c) {
		case 0:
			UpdatePositionDampened((position + amb2) * .5f);
			hasValidPosition = true;
			break;
		case 1:
			UpdatePositionDampened(amb1);
			hasValidPosition = true;
			break;
		case 2:
			//Needs better check?
			if (glm::length2(position - amb1) < glm::length2(position - amb2))
				UpdatePositionDampened(amb1);
			else
				UpdatePositionDampened(amb2);
			hasValidPosition = true;
			break;
		}
	}
	else if (trackers[shoulder].hasValidPosition) {
		uint8_t c = CalculateOneValid(trackers[shoulder].position, lowerLegLen, cameras[n].position, directions[n]);
		switch (c) {
		case 0:
			UpdatePositionDampened((position + amb2) * .5f);
			hasValidPosition = true;
			break;
		case 1:
			UpdatePositionDampened(amb1);
			hasValidPosition = true;
			break;
		case 2:
			//Needs better check?
			if (glm::length2(position - amb1) < glm::length2(position - amb2))
				UpdatePositionDampened(amb1);
			else
				UpdatePositionDampened(amb2);
			hasValidPosition = true;
			break;
		}
	}
	else if (trackers[wrist].hasValidPosition) {
		uint8_t c = CalculateOneValid(trackers[wrist].position, lowerLegLen, cameras[n].position, directions[n]);
		switch (c) {
		case 0:
			UpdatePositionDampened((position + amb2) * .5f);
			hasValidPosition = true;
			break;
		case 1:
			UpdatePositionDampened(amb1);
			hasValidPosition = true;
			break;
		case 2:
			//Needs better check?
			if (glm::length2(position - amb1) < glm::length2(position - amb2))
				UpdatePositionDampened(amb1);
			else
				UpdatePositionDampened(amb2);
			hasValidPosition = true;
			break;
		}
	}
}

//vec3 getFootForward(foot, knee, hip)
//for straight legs, uses angle lerp and hip orientation
//what's the equation for cross product, can that convert directly to angle?
//used for foot and knee, when dot is too high or low |d|>.9[5]f
//reject foot - hip with up (compared to world or hip) vector for direction
//cross/dot/however we get angle with forward/right
//if sufficiently up or down (compared to hip) use hip forward
//then take orthoganal to leg closest to up,
//and rotate it along the leg by the given angle
glm::vec3 PoseTracker::GetFootForward(uint8_t foot, uint8_t knee, uint8_t hip) {
	glm::vec3 hipLeft = glm::normalize(trackers[Poses::left_hip].position - trackers[Poses::right_hip].position);
	glm::vec3 hipUp = glm::normalize((trackers[Poses::left_shoulder].position + trackers[Poses::right_shoulder].position) 
		- (trackers[Poses::left_hip].position + trackers[Poses::right_hip].position));
	glm::vec3 hipForward = glm::normalize(glm::cross(hipLeft, hipUp));

	glm::vec3 hipToFoot = trackers[foot].position - trackers[hip].position;

	glm::vec3 footToKnee = glm::normalize(trackers[knee].position - trackers[foot].position);
	float hipKneeDot = glm::dot(-footToKnee, glm::normalize(hipToFoot));
	if (hipKneeDot > .85f) {
		//perform verticality check
		float footVecDot = glm::dot(glm::normalize(hipToFoot), hipUp);
		if (fabsf(footVecDot) > .95f) {
			return glm::normalize(reject(hipForward, hipToFoot)) * (footVecDot > 0 ? -1.0f : 1.0f);
		}

		glm::vec3 footDir = glm::normalize(reject(hipToFoot, hipUp));
		glm::vec3 footCross = glm::cross(footDir, hipForward);
		float theta = asinf(glm::length(footCross)) * (glm::dot(footCross, hipUp) >= 0 ? -1 : 1);
		glm::quat rot = glm::angleAxis(theta, glm::normalize(hipToFoot));

		glm::vec3 footForward = glm::normalize(reject(hipUp, hipToFoot));
		footForward = rot * footForward;
		return footForward;
	}
	//else if(hipKneeDot < -.9f) //figure this out
	else {
		glm::vec3 left = glm::normalize(glm::cross(-footToKnee, trackers[hip].position - trackers[knee].position));
		return glm::normalize(glm::cross(left, footToKnee));
	}
}

void PoseTracker::CalculateOrientationAnkle(uint8_t knee, uint8_t hip) {
	//use floor(level) when on or below
	//put toes on when near (<60/70 deg)
	//use knee when above

	//when off floor, use plane between hip, knee, and foot for orientation
	//when on floor, have setting to switch between that and current (using hip forward)

	bool footFollowHip = true;

	float height = position.y;// -ankleToSole;
	float delh = height - maxAnkle;
	float dot = glm::dot(glm::normalize(trackers[knee].position - position), glm::vec3(0, 1, 0));
	if (height <= maxAnkle && dot > .95f) {
		//if dot between up and knee is too small, do same as else statement
		if (footFollowHip) {
			rotation = glm::quatLookAt(reject(trackers[right_hip].rotation * glm::vec3(1, 0, 0),
				glm::vec3(0, 1, 0)), glm::vec3(0, 1, 0));
		}
		else {
			glm::vec3 up(0, 1, 0);
			glm::vec3 forward = GetFootForward(tracker, knee, hip);
			forward = glm::normalize(reject(forward, up));
			rotation = glm::quatLookAt(forward, up);
		}
		if (delh > 0) {
			float theta = asinf(delh / footLen);
			glm::quat footRot = glm::angleAxis(theta, glm::vec3(-1, 0, 0));
			rotation = footRot * rotation;
		}
	}
	else {
		glm::vec3 up = glm::normalize(trackers[knee].position - position);
		glm::vec3 forward = GetFootForward(tracker, knee, hip);
		rotation = glm::quatLookAt(forward, up);
	}
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

	case Poses::left_ankle:
		CalculateOrientationAnkle(Poses::left_knee, Poses::left_hip);
		break;
	case Poses::right_ankle:
		CalculateOrientationAnkle(Poses::right_knee, Poses::right_hip);
		break;

		//Hip
	case Poses::left_hip:
		rotation = glm::quatLookAt(glm::normalize(trackers[Poses::right_hip].position - position),
			glm::normalize(trackers[Poses::left_shoulder].position - position));
		break;
	case Poses::right_hip:
		rotation = glm::quatLookAt(glm::normalize(trackers[Poses::left_hip].position - position),
			glm::normalize(trackers[Poses::right_shoulder].position - position));
		break;

		// Elbow / Knee
	case Poses::left_elbow:
		rotation = glm::quatLookAt(glm::normalize(trackers[Poses::left_wrist].position - position),
			-glm::normalize(trackers[Poses::left_shoulder].position - position));
		break;
	case Poses::right_elbow:
		rotation = glm::quatLookAt(glm::normalize(trackers[Poses::right_wrist].position - position),
			-glm::normalize(trackers[Poses::right_shoulder].position - position));
		break;
	case Poses::left_knee:
		rotation = glm::quatLookAt(glm::normalize(trackers[Poses::left_ankle].position - position),
			-glm::normalize(trackers[Poses::left_hip].position - position));
		break;
	case Poses::right_knee:
		rotation = glm::quatLookAt(glm::normalize(trackers[Poses::right_ankle].position - position),
			-glm::normalize(trackers[Poses::right_hip].position - position)); //Maybe flip sign based on headset/hip
		break;


	case Poses::left_shoulder:
		rotation = glm::quatLookAt(glm::normalize(trackers[Poses::left_hip].position - position),
			glm::normalize(trackers[Poses::right_shoulder].position - position));
		break;
	case Poses::right_shoulder:
		rotation = glm::quatLookAt(glm::normalize(trackers[Poses::right_hip].position - position),
			glm::normalize(trackers[Poses::left_shoulder].position - position));
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
	VRDashboardOverlay::SharedInstance()->SetCameraState(index, CameraState::Camera_Connecting);
	cameras[index].active = false;
	cameras[index].connected = true;
	CreateCameraOverlay(index);
}
void Camera::OnDisconnect(uint8_t index) {
	VRDashboardOverlay::SharedInstance()->SetCameraState(index, CameraState::Camera_Inactive);
	cameras[index].active = false;
	cameras[index].connected = false;
}
void Camera::OnStart(uint8_t index) {
	VRDashboardOverlay::SharedInstance()->SetCameraState(index, CameraState::Camera_NeedsCalibration);
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

	float ats = fabsf((positions[Poses::left_ankle].y + positions[Poses::right_ankle].y) * .5f);
	ankleToSole = (ankleToSole * distancesRecorded + ats) / (distancesRecorded + 1);
	float sts = glm::length(positions[Poses::left_shoulder] - positions[Poses::right_shoulder]);
	shoulderToShoulder = (shoulderToShoulder * distancesRecorded + sts) / (distancesRecorded + 1);
	float hw = glm::length(positions[Poses::left_hip] - positions[Poses::right_hip]);
	hipWidth = (hipWidth * distancesRecorded + hw) / (distancesRecorded + 1);
	float sth = (glm::length(positions[Poses::left_hip] - positions[Poses::left_shoulder]) + 
		glm::length(positions[Poses::right_hip] - positions[Poses::right_shoulder])) * .5f;
	shoulderToHip = (shoulderToHip * distancesRecorded + sth) / (distancesRecorded + 1);
	float ua = (glm::length(positions[Poses::left_elbow] - positions[Poses::left_shoulder]) +
		glm::length(positions[Poses::right_elbow] - positions[Poses::right_shoulder])) * .5f;
	upperArmLen = (upperArmLen * distancesRecorded + ua) / (distancesRecorded + 1);
	float la = (glm::length(positions[Poses::left_elbow] - positions[Poses::left_wrist]) +
		glm::length(positions[Poses::right_elbow] - positions[Poses::right_wrist])) * .5f;
	lowerArmLen = (lowerArmLen * distancesRecorded + la) / (distancesRecorded + 1);
	float ul = (glm::length(positions[Poses::left_hip] - positions[Poses::left_knee]) +
		glm::length(positions[Poses::right_hip] - positions[Poses::right_knee])) * .5f;
	upperLegLen = (upperLegLen * distancesRecorded + ul) / (distancesRecorded + 1);
	float ll = (glm::length(positions[Poses::left_ankle] - positions[Poses::left_knee]) +
		glm::length(positions[Poses::right_ankle] - positions[Poses::right_knee])) * .5f;
	lowerLegLen = (lowerLegLen * distancesRecorded + ll) / (distancesRecorded + 1);
	footLen = lowerLegLen * footToLowerLegRatio;
	maxAnkle = footLen * .5f;
	distancesRecorded++;

	/*std::cout << "ankleToSole        " << ankleToSole << std::endl;
	std::cout << "shoulderToShoulder " << shoulderToShoulder << std::endl;
	std::cout << "hipWidth           " << hipWidth << std::endl;
	std::cout << "shoulderToHip      " << shoulderToHip << std::endl;
	std::cout << "upperArmLen        " << upperArmLen << std::endl;
	std::cout << "lowerArmLen        " << lowerArmLen << std::endl;
	std::cout << "upperLegLen        " << upperLegLen << std::endl;
	std::cout << "lowerLegLen        " << lowerLegLen << std::endl << std::endl;*/
}