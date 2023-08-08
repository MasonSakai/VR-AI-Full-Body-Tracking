#pragma once

#include "vrUtil.h"

extern glm::vec3 pmOffset;
extern uint8_t pmFlags;

void EnableHardwareOffset();
void UpdateHardwareOffset();
void DisableHardwareOffset();

void CheckPlayspaceMover();

enum PlayspaceMoverFlags : uint8_t {
	None = 0,
	Active = 128,
	Moving = 1,
	ControllerRight = 2,
	DoubleButtonReset = 4,
	DoubleButtonResetting = 8
};