#pragma once

#include "cameraSocketManager.h"
#include <qobject.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qhttpserver.h>
#include <QDebug>

extern QString BaseDirectory;
extern QJsonDocument config;

QByteArray ReadFile(QString dir);
void WriteFile(QString dir, QByteArray data);

bool ReadConfig();
void WriteConfig();

class AIRemoteServer : public QObject {
	Q_OBJECT
public:

	static AIRemoteServer* SharedInstance();

	bool StartServer();

signals:

public slots:
	void onConnect();

private:
	QHttpServer* server;
};

