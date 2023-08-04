#pragma once

#include "CameraManager.h"
#include "PoseTracker.h"
#include "Config.h"
#include "DashboardWidget.h"

#include <qhttpserver.h>
#include <qobject.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>

class AIRemoteServer : public QObject {
	Q_OBJECT
public:

	static AIRemoteServer* SharedInstance();

	bool StartServer();

private:
	QHttpServer* server;
};

