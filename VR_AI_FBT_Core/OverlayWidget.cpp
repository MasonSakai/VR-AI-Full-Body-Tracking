#include "OverlayWidget.h"

QString labelBase = "lblLine";

const int pxPerChar = 8,
minCharPerLine = 25,
maxCharPerLine = 50,
pxPerLine = 24,
marginWidth = 50,
marginHeight = 20;

OverlayWidget::OverlayWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	for (int i = 0; i < 3; i++) {
		QString labelName = labelBase;
		labelName.append(QString::number(i));
		lblLines[i] = findChild<QLabel*>(labelName);
		lblLines[i]->setText(labelName);
	}
}

OverlayWidget::~OverlayWidget()
{}



void OverlayWidget::SetLine(uint8_t index, QString text) {
	lblLines[index]->setText(text);
}
void OverlayWidget::SetText(QString text) {
	SetLine(0, text);
}
