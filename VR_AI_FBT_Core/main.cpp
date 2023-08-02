
//If you begin to have issues with boost libraries
//https://stackoverflow.com/questions/39363720/compiling-boost-with-msvc2015-with-stdclatest-or-c17-n4190

#include "DashboardWidget.h"
#include "VRDashboardOverlay.h"
#include "PoseTracker.h"
#include "websiteServer.h"
#include "vrUtil.h"
#include "CameraManager.h"
#include "OverlayManager.h"
#include "Config.h"

#include <QApplication>
#include <QDebug>
#include <Windows.h>
#include <iostream>
#include <thread>

std::thread MainThread;

void MainLoop() {
	active = true;

	std::cout << "Setting Cameras\n";
	VRDashboardOverlay::SharedInstance()->SetCameraState(0, 0);
	VRDashboardOverlay::SharedInstance()->SetCameraState(1, 1);
	VRDashboardOverlay::SharedInstance()->SetCameraState(2, 2);

	uint8_t trackerStates[17];
	bool singleTrackerStates[17];
	uint8_t i, j;
	bool completed;

	auto lastTime = std::chrono::high_resolution_clock::now();
	int numFramePresents = 0;
	float deltaTime;
	uint32_t currentFrame = 0;

	while (active) {
		if (vr::VRCompositor() != NULL) {
			vr::Compositor_FrameTiming t;
			t.m_nSize = sizeof(vr::Compositor_FrameTiming);
			bool hasFrame = vr::VRCompositor()->GetFrameTiming(&t, 0);
			auto currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> dt = currentTime - lastTime;
			deltaTime = dt.count();
			// If the frame has changed we update, if a frame was redisplayed we update.
			if ((hasFrame && currentFrame != t.m_nFrameIndex) || (hasFrame && t.m_nNumFramePresents != numFramePresents)) {
				currentFrame = t.m_nFrameIndex;
				numFramePresents = t.m_nNumFramePresents;
				lastTime = currentTime;

				UpdateHardwarePositions();
				UpdateRealHardwarePositions();

				if (pmFlags & PlayspaceMoverFlags::Active) {
					CheckPlayspaceMover();
				}

				//Do Math
				for (i = 0; i < 17; i++) {
					singleTrackerStates[i] = false;
					trackerStates[i] = trackers[i].CalculatePosition();
				}

				for (j = 0; j < 10; j++) {
					completed = true;
					for (i = 0; i < 17; i++) {
						if (trackerStates[i] < 2 && !singleTrackerStates[i]) {
							if (trackerStates[i] == 1) {
								singleTrackerStates[i] = trackers[i].CalculateSingleCameraPosition();
								completed &= singleTrackerStates[i];
							}
							else {
								//todo
							}
						}
					}
					if (completed) break;
				}

				for (i = 0; i < 17; i++) {
					if (trackerStates[i] == 2 || singleTrackerStates[i])
						trackers[i].CalculateOrientation();
				}

				if (!trackersOverride)
					for (i = 0; i < 17; i++) {
						if (PoseTrackers[i]) {
							setOffsetVirtualDevicePosition(trackerIDs[i], trackers[i].position, trackers[i].rotation);
						}
					}

				// Sleep for just under 1/90th of a second, so that maybe the next frame will be available.
				std::this_thread::sleep_for(std::chrono::microseconds(10000));
			}
			else {
				// Still waiting on the next frame, wait less this time.
				std::this_thread::sleep_for(std::chrono::microseconds(1111));
			}
		}

	}
}


void HandleArgs(int argc, char* argv[]) {
	if (argc < 2) return;
	for (int i = 1; i < argc; i++) {
		std::cout << argv[i];

		std::cout << std::endl << std::flush;
	}
}

void endProgram() {
	std::cout << "Driver Stopping...\n" << std::flush;
	active = false;
	calibrating = false;
	OverlayOnClose();
	
	MainThread.join();
	if (pmFlags & PlayspaceMoverFlags::Active) {
		pmOffset = glm::vec3();
		DisableHardwareOffset();
	}

	if (calibrating && CalibrationThread != nullptr)
		CalibrationThread->join();

	//Save data if needed
	WriteConfig();
	std::cout << "Driver Stopped\n" << std::flush;
}

static bool EndProgram(DWORD signal) {
	switch (signal)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		std::cout << "Closing..." << std::endl;
		QApplication::quit();
		return true;

	default:
		return false;
	}
}


int main(int argc, char* argv[])
{
#ifdef _WIN32
	if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
	freopen("out.txt", "w", stdout);
	freopen("out.txt", "w", stderr);
#endif
	
	HandleArgs(argc, argv);
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)EndProgram, true);

	QApplication a(argc, argv);

	BaseDirectory.append("C:\\VSProjects\\VR-AI-Full-Body-Tracking\\Remote1CamProcessing\\");
	if (QApplication::arguments().contains("-webDirectory")) {
		int index = QApplication::arguments().indexOf("-webDirectory");
		BaseDirectory = QApplication::arguments().at(index + 1);
	}

	if (!ReadConfig()) {
		QJsonObject json;
		json.insert("port", 2674);
		json.insert("windowConfigs", QJsonArray());
		config.setObject(json);
		WriteConfig();
	}

	StartVR();
	VRDashboardOverlay::SharedInstance();

	MainThread = std::thread(MainLoop);

	AIRemoteServer::SharedInstance()->StartServer();
	int i = a.exec();
	endProgram();
	return i;
}
