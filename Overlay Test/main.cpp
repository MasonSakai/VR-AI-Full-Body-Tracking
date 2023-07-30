#include "overlaywidget.h"
#include "openvroverlaycontroller.h"
#include <QApplication>

#include <Windows.h>
#include <iostream>
#include <stdio.h>

int main(int argc, char *argv[])
{
#ifdef _WIN32
	if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) { //this doesn't work...
		freopen("CONOUT$", "w", stdout);
		freopen("CONERR$", "w", stderr);
	}
#endif
	std::cout << "Test\n";

	QApplication a(argc, argv);
	OverlayWidget *pOverlayWidget = new OverlayWidget();

	COpenVROverlayController::SharedInstance()->Init();

	COpenVROverlayController::SharedInstance()->SetWidget( pOverlayWidget );

	// don't show widgets that you're going display in an overlay
	//w.show();

	return a.exec();
}
