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
		freopen("CONIN$", "r", stdin);
	}
#endif
	std::cout << "Test\n";

	QApplication a(argc, argv);
	OverlayWidget *pOverlayWidget = new OverlayWidget();

	COpenVROverlayController::SharedInstance()->Init();

	COpenVROverlayController::SharedInstance()->SetWidget( pOverlayWidget );

	// don't show widgets that you're going display in an overlay
	//w.show();
	
	int i = a.exec();
	char c;
	while (true) {
		std::cin >> c;
		std::cout << c;
	}

	return i;
}
