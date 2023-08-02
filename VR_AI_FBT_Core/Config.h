#pragma once

#include <qobject.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>

extern QString BaseDirectory;
extern QJsonDocument config;

QByteArray ReadFile(QString dir);
void WriteFile(QString dir, QByteArray data);

bool ReadConfig();
void WriteConfig();