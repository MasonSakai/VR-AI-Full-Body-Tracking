#pragma once

#include <QWidget>
#include "ui_OverlayWidget.h"
#include <qlabel.h>
#include <qstring.h>
#include <string.h>

class OverlayWidget : public QWidget
{
	Q_OBJECT

public:
	OverlayWidget(QWidget* parent = nullptr);
	~OverlayWidget();

	void SetLine(uint8_t index, QString text);
	void SetText(QString text);

private:
	Ui::OverlayWidgetClass ui;

	QLabel *lblLines[3];
};

