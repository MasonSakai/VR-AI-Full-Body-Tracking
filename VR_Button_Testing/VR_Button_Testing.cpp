
#include "vrUtil.h"
#include <Windows.h>

bool active = true;

static bool EndProgram(DWORD signal) {
	switch (signal)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		std::cout << "Closing..." << std::endl;
		active = false;
		return true;

	default:
		return false;
	}
}

int main()
{

    StartVR();

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)EndProgram, true);

	std::cout << "Press ctrl+c or close the console to stop";

	uint64_t left, right;

	while (active) {
		left = GetControllerState(vr::TrackedControllerRole_LeftHand).ulButtonPressed;
		right = GetControllerState(vr::TrackedControllerRole_RightHand).ulButtonPressed;
        std::cout << left << "\n" << right << "\n\n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    DisconnectFromVRRuntime();

	std::cout << "Press Enter to close...";
	std::cin >> left;
    return 0;
}
