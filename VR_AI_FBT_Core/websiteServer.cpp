#include "websiteServer.h"
#include <QMimeDatabase>
#include <qhttpserver.h>
#include <qfile.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qapplication.h>
#include <qtextstream.h>

QHttpServer server;
QString BaseDirectory;
QJsonDocument config;


QByteArray ReadFile(QString dir) {
	QFile file(dir);
	QByteArray bytes;
	if (file.open(QIODevice::ReadOnly)) {
		bytes = file.readAll();
		file.close();
	}
	return bytes;
}
void WriteFile(QString dir, QByteArray data) {
	QFile file(dir);
	QByteArray bytes;
	if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		QTextStream iStream(&file);
		iStream.setEncoding(QStringConverter::Encoding::Utf8);
		iStream << bytes;
		file.close();
	}
}

bool ReadConfig() {
	QString fileName = BaseDirectory;
	fileName.append("config.json");
	QByteArray data = ReadFile(fileName);
	QJsonParseError jsonError;
	config = QJsonDocument::fromJson(data, &jsonError);
	if (jsonError.error != QJsonParseError::NoError)
	{
		return false;
	}
	return true;
}
void WriteConfig() {
	QString fileName = BaseDirectory;
	fileName.append("config.json");
	QByteArray data = config.toJson(QJsonDocument::Indented);
	WriteFile(fileName, data);
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


bool StartServer() {

	BaseDirectory.append("C:\\VSProjects\\VR-AI-Full-Body-Tracking\\Remote1CamProcessing\\");
	if (QApplication::arguments().contains("-webDirectory")) {
		int index = QApplication::arguments().indexOf("-webDirectory");
		BaseDirectory = QApplication::arguments().at(index + 1);
	}

	if (!ReadConfig()) {
		
	}

	server.route("/", QHttpServerRequest::Method::Get, defaultHandler);
	server.route("/config.json", configHandler);
	server.setMissingHandler(missingHandler);

	server.listen(QHostAddress::Any, 2674);
	return true;
}
bool StopServer() {
	return true;
}