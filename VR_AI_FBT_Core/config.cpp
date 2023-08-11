#include "Config.h"
#include <qfile.h>

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
	if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		QTextStream iStream(&file);
		iStream.setEncoding(QStringConverter::Encoding::Utf8);
		iStream << data;
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
void InitConfig() {
	QJsonObject json;
	json.insert("port", 2674);
	json.insert("trackerDampening", .9f);

	QJsonArray windowConfigs;
	QJsonObject windowConfig;
	windowConfig.insert("cameraName", "");
	windowConfig.insert("autostart", false);
	windowConfig.insert("confidenceThreshold", .3);
	windowConfigs.append(windowConfig);
	json.insert("windowConfigs", windowConfigs);

	QJsonObject pmConfig;
	pmConfig.insert("enabled", true);
	pmConfig.insert("doubleButtonReset", true);
	QJsonObject pmButtons;
	pmButtons.insert("oculusax", false);
	pmButtons.insert("oculusby", true);
	pmButtons.insert("oculustrigger", false);
	pmButtons.insert("oculusbumper", false);
	pmButtons.insert("maskRaw", 0);
	pmConfig.insert("buttons", pmButtons);
	json.insert("PlayspaceMover", pmConfig);

	QJsonObject trackers;
	trackers.insert("ankle", true);
	trackers.insert("knee", false);
	trackers.insert("hip", true);
	trackers.insert("shoulder", false);
	trackers.insert("elbow", false);
	json.insert("trackers", trackers);

	QJsonObject buttons;
	buttons.insert("oculusax", true);
	buttons.insert("oculusby", true);
	buttons.insert("oculustrigger", false);
	buttons.insert("oculusbumper", false);
	buttons.insert("maskRaw", 0);
	json.insert("buttons", buttons);

	config.setObject(json);
	WriteConfig();
}

/* Example Config:
"port": 2674,
"windowConfigs": [
	{
		"cameraName": "USB2.0 HD UVC WebCam (13d3:5666)",
		"autostart": false,
		"confidenceThreshold": 0.3
	},
	{
		"cameraName": "DroidCam Source 2",
		"autostart": false,
		"confidenceThreshold": 0.3
	}
],
"PlayspaceMover": {
	"enabled": true,
	"doubleButtonReset": true,
	"buttons": {
		"oculusax": false,
		"oculusby": true,
		"oculustrigger": false,
		"oculusbumper": false
	}
},
"trackers": {
	"ankle": true,
	"knee": false,
	"hip": true,
	"shoulder": false,
	"elbow": false
},
"buttons": {
	"oculusax": true,
	"oculusby": true,
	"oculustrigger": false,
	"oculusbumper": false
}
*/

