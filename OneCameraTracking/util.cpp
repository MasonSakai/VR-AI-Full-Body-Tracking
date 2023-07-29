#include "util.h"

vr::HmdQuaternion_t GetRotation(vr::HmdMatrix34_t matrix) {
	vr::HmdQuaternion_t q;

	q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
	q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
	q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
	return q;
}
glm::quat GetRotationGLM(vr::HmdMatrix34_t matrix) {
	glm::quat q;

	q.w = sqrtf(fmaxf(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = sqrtf(fmaxf(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.y = sqrtf(fmaxf(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.z = sqrtf(fmaxf(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
	q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
	q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
	return q;
}
glm::quat GetRotationGLM(vr::HmdQuaternion_t quat) {
	glm::quat q;
	q.x = quat.x;
	q.y = quat.y;
	q.z = quat.z;
	q.w = quat.w;
	return q;
}

vr::HmdVector3_t GetPosition(vr::HmdMatrix34_t matrix) {
	vr::HmdVector3_t vector;

	vector.v[0] = matrix.m[0][3];
	vector.v[1] = matrix.m[1][3];
	vector.v[2] = matrix.m[2][3];

	return vector;
}
glm::vec3 GetPositionGLM(vr::HmdMatrix34_t matrix) {
	glm::vec3 vector;

	vector.x = matrix.m[0][3];
	vector.y = matrix.m[1][3];
	vector.z = matrix.m[2][3];

	return vector;
}
glm::vec3 GetPositionGLM(vr::HmdVector3d_t vec) {
	glm::vec3 vector;

	vector.x = vec.v[0];
	vector.y = vec.v[1];
	vector.z = vec.v[2];

	return vector;
}

glm::vec3 lerp(glm::vec3 v1, glm::vec3 v2, float t) {
	return v1 * (1 - t) + v2 * t;
}
glm::vec2 lerp(glm::vec2 v1, glm::vec2 v2, float t) {
	return v1 * (1 - t) + v2 * t;
}

glm::vec3 project(glm::vec3 v1, glm::vec3 v2) {
	return glm::dot(v1, v2) * v2 / glm::length2(v2);
}
glm::vec2 project(glm::vec2 v1, glm::vec2 v2) {
	return glm::dot(v1, v2) * v2 / glm::length2(v2);
}
glm::vec3 reject(glm::vec3 v1, glm::vec3 v2) {
	return v1 - project(v1, v2);
}
glm::vec2 reject(glm::vec2 v1, glm::vec2 v2) {
	return v1 - project(v1, v2);
}

glm::vec3 Intersection(glm::vec3 p1, glm::vec3 v1, glm::vec3 p2, glm::vec3 v2) {
	glm::mat3x3 mat;
	mat[0][0] = v1.x;
	mat[1][0] = v1.y;
	mat[2][0] = v1.z;

	mat[0][1] = -v2.x;
	mat[1][1] = -v2.y;
	mat[2][1] = -v2.z;

	mat[0][2] = p1.x - p2.x;
	mat[1][2] = p1.y - p2.y;
	mat[2][2] = p1.z - p2.z;

	/*std::cout << "{" << mat[0][0] << ", " << mat[0][1] << ", " << mat[0][2] << "}\n";
	std::cout << "{" << mat[1][0] << ", " << mat[1][1] << ", " << mat[1][2] << "}\n";
	std::cout << "{" << mat[2][0] << ", " << mat[2][1] << ", " << mat[2][2] << "}\n\n" << std::flush;*/

	float s = mat[0][0];
	mat[0][0] /= s;
	mat[0][1] /= s;
	mat[0][2] /= s;

	for (int i = 1; i < 3; i++) {
		s = mat[i][0];
		for (int j = 0; j < 3; j++) {
			mat[i][j] -= mat[0][j] * s;
			if (fabsf(mat[i][j]) < 0.001f) mat[i][j] = 0;
		}
	}

	s = mat[1][1];
	mat[1][1] /= s;
	mat[1][2] /= s;

	s = mat[2][1];
	for (int j = 1; j < 3; j++) {
		mat[2][j] -= mat[1][j] * s;
		if (fabsf(mat[2][j]) < 0.001f) mat[2][j] = 0;
	}

	/*std::cout << "{" << mat[0][0] << ", " << mat[0][1] << ", " << mat[0][2] << "}\n";
	std::cout << "{" << mat[1][0] << ", " << mat[1][1] << ", " << mat[1][2] << "}\n";
	std::cout << "{" << mat[2][0] << ", " << mat[2][1] << ", " << mat[2][2] << "}\n\n" << std::flush;*/

	if (mat[2][2] != 0) return glm::vec3(INFINITY);

	s = mat[1][2];
	return p2 + v2 * s;
}

glm::vec3 IntersectPlane(glm::vec3 planePos, glm::vec3 planeNorm, glm::vec3 startPos, glm::vec3 direction) {
	return glm::vec3();
}

vr::HmdMatrix34_t ConvertMatrix(glm::mat4x4 matrix, glm::vec3 position) {
	vr::HmdMatrix34_t mat;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			mat.m[i][j] = matrix[i][j];

	mat.m[0][3] = position.x;
	mat.m[1][3] = position.y;
	mat.m[2][3] = position.z;

	return mat;
}