#include "websiteServer.h"
#include <QMimeDatabase>
#include <qapplication.h>

AIRemoteServer* sharedServer = NULL;
AIRemoteServer* AIRemoteServer::SharedInstance() {
	if (!sharedServer) {
		sharedServer = new AIRemoteServer();
	}
	return sharedServer;
}


QHttpServerResponse defaultHandler(const QHttpServerRequest& req) {
	QString fileName = BaseDirectory;
	fileName.append("dist\\index.html");
	QByteArray data = ReadFile(fileName);
	return QHttpServerResponse("text/html", data);
}
void configHandler(const QHttpServerRequest& req, QHttpServerResponder&& res) {
	int index = 0;
	QByteArray data;
	QJsonParseError jsonError;
	QJsonDocument jsonDocument;
	QJsonObject jsonObject;
	switch (req.method())
	{
	case QHttpServerRequest::Method::Get:
		data = req.body();
		index = data.toInt(0);
		if (index == -1) index = 0; //Make better
		qDebug() << "Get Config " << index;
		jsonObject = config["windowConfigs"].toArray()[index].toObject();
		jsonObject.insert("status", "ok");
		res.sendResponse(QHttpServerResponse(jsonObject));
		return;
	case QHttpServerRequest::Method::Post:
		
		data = req.body();
		jsonDocument = QJsonDocument::fromJson(data, &jsonError);
		if (jsonError.error != QJsonParseError::NoError || !jsonDocument.isObject())
		{
			res.sendResponse(QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest));
			return;
		}
		index = jsonDocument.object()["id"].toInt();
		jsonDocument.object().remove("id");
		config["windowConfigs"][index].toObject() = jsonDocument.object();
		WriteConfig();
		res.sendResponse(QHttpServerResponse(QHttpServerResponder::StatusCode::Ok));
		return;
	default:
		res.sendResponse(QHttpServerResponse(QHttpServerResponder::StatusCode::MethodNotAllowed));
		return;
	}
}
void missingHandler(const QHttpServerRequest& req, QHttpServerResponder&& res) {
	QString fileName = BaseDirectory;
	fileName.append("dist\\");
	fileName.append(req.url().fileName().toStdString());
	QByteArray data = ReadFile(fileName);
	QString type = QMimeDatabase().mimeTypeForFile(req.url().fileName()).name();
	res.sendResponse(QHttpServerResponse(type.toStdString().c_str(), data));
}

void onPoseData(const QHttpServerRequest& req, QHttpServerResponder&& res) {
	QByteArray data = req.body();
	QJsonParseError jsonError;
	QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &jsonError);
	if (jsonError.error != QJsonParseError::NoError || !jsonDocument.isObject())
	{
		res.sendResponse(QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest));
		return;
	}
	int index = jsonDocument.object()["id"].toInt();
	QJsonObject pose = jsonDocument.object()["pose"].toObject();
	qDebug() << jsonDocument << index;
	PoseTracker::SetPose(index, pose);
	res.sendResponse(QHttpServerResponse(QHttpServerResponder::StatusCode::Ok));
}
void onGetSize(const QHttpServerRequest& req, QHttpServerResponder&& res) {
	QByteArray data = req.body();
	QJsonParseError jsonError;
	QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &jsonError);
	if (jsonError.error != QJsonParseError::NoError || !jsonDocument.isObject())
	{
		res.sendResponse(QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest));
		return;
	}
	int index = jsonDocument.object()["id"].toInt();
	int width = jsonDocument.object()["width"].toInt();
	int height = jsonDocument.object()["height"].toInt();
	Camera::SetSize(index, width, height);
	qDebug() << jsonDocument << index;
	res.sendResponse(QHttpServerResponse(QHttpServerResponder::StatusCode::Ok));
}

bool AIRemoteServer::StartServer() {
	server = new QHttpServer(this);

	server->route("/", QHttpServerRequest::Method::Get, defaultHandler);
	server->route("/config.json", configHandler);
	server->route("/poseData", onPoseData);
	server->route("/cameraSize", onGetSize);
	server->setMissingHandler(missingHandler);

	server->listen(QHostAddress::Any, config["port"].toInt());
	return true;
}