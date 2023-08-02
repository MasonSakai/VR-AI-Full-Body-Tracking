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
	return; //fix this
	QString fileName = BaseDirectory;
	fileName.append("config.json");
	QByteArray data = config.toJson(QJsonDocument::Indented);
	WriteFile(fileName, data);
}


