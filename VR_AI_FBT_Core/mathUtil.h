#pragma once

#include <openvr.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

vr::HmdQuaternion_t GetRotation(vr::HmdMatrix34_t matrix);
glm::quat GetRotationGLM(vr::HmdMatrix34_t matrix);
glm::quat GetRotationGLM(vr::HmdQuaternion_t quat);

vr::HmdVector3_t GetPosition(vr::HmdMatrix34_t matrix);
glm::vec3 GetPositionGLM(vr::HmdMatrix34_t matrix);
glm::vec3 GetPositionGLM(vr::HmdVector3d_t vec);

vr::HmdMatrix34_t ConvertMatrix(glm::mat4x4 matrix, glm::vec3 position);

glm::vec3 lerp(glm::vec3 v1, glm::vec3 v2, float t);
glm::vec2 lerp(glm::vec2 v1, glm::vec2 v2, float t);

//Projects v1 onto v2
glm::vec3 project(glm::vec3 v1, glm::vec3 v2);
glm::vec2 project(glm::vec2 v1, glm::vec2 v2);
glm::quat project(glm::quat q, glm::vec3 v);
glm::vec3 reject(glm::vec3 v1, glm::vec3 v2);
glm::vec2 reject(glm::vec2 v1, glm::vec2 v2);

glm::vec3 Intersection(glm::vec3 p1, glm::vec3 v1, glm::vec3 p2, glm::vec3 v2);

glm::vec3 IntersectPlane(glm::vec3 planePos, glm::vec3 planeNorm, glm::vec3 startPos, glm::vec3 direction);

uint8_t IntersectSphere(glm::vec3 center, float radius, glm::vec3 startPos, glm::vec3 direction, glm::vec3* p1, glm::vec3* p2, float tolerance = .001f);