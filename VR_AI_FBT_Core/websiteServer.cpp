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

int connected = 0;

QHttpServerResponse defaultHandler(const QHttpServerRequest& req) {
	QString fileName = BaseDirectory;
	fileName.append("dist\\index.html");
	QByteArray data = ReadFile(fileName);
	return QHttpServerResponse("text/html", data);
}
void configHandler(const QHttpServerRequest& req, QHttpServerResponder&& res) {
	int index = -1;
	QByteArray data;
	QJsonParseError jsonError;
	QJsonDocument jsonDocument;
	QJsonArray jsonArray;
	QJsonObject jsonObject;
	switch (req.method())
	{
	case QHttpServerRequest::Method::Get:
		data = req.body();
		if(!data.isEmpty())
			index = data.toInt();
		if (index == -1) {
			index = connected; //Make better
			connected++;
		}
		jsonArray = config["windowConfigs"].toArray();
		if (index < jsonArray.count() && jsonArray[index].isObject()) {
			jsonObject = jsonArray[index].toObject();
			jsonObject.insert("status", "ok");
			jsonObject.insert("id", index);
			res.sendResponse(QHttpServerResponse(jsonObject));
		}
		else
		{
			jsonObject = QJsonObject();
			if (index != 0) {
				jsonObject = jsonArray[index - 1].toObject();
				jsonArray.append(jsonObject);
			}
			else {
				jsonObject.insert("autostart", false);
				jsonObject.insert("confidenceThreshold", 0.3f);
				jsonObject.insert("cameraName", "");
				jsonArray.append(jsonObject);
			}
			jsonObject.insert("status", "madeConfig");
			jsonObject.insert("id", index);
			res.sendResponse(QHttpServerResponse(jsonObject));
		}
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
	res.sendResponse(QHttpServerResponse(QHttpServerResponder::StatusCode::Ok));
}
void onCameraConnect(const QHttpServerRequest& req, QHttpServerResponder&& res) {
	int index = req.body().toInt();
	Camera::OnConnect(index);
	VRFloatingOverlay::SharedInstance()->QueueText("Camera Connected", 1.5f);
}
void onCameraStart(const QHttpServerRequest& req, QHttpServerResponder&& res) {
	int index = req.body().toInt();
	Camera::OnStart(index);
	VRFloatingOverlay::SharedInstance()->QueueText("Camera Awaiting Calibration", 1.5f);
}

bool AIRemoteServer::StartServer() {
	server = new QHttpServer(this);

	server->route("/", QHttpServerRequest::Method::Get, defaultHandler);
	server->route("/config", configHandler);
	server->route("/poseData", onPoseData);
	server->route("/cameraSize", onGetSize);
	server->route("/connect", onCameraConnect);
	server->route("/start", onCameraStart);
	server->setMissingHandler(missingHandler);

	server->listen(QHostAddress::Any, config["port"].toInt());
	return true;
}