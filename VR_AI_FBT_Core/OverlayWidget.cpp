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
		labelName.append(std::to_string(i));
		lblLines[i] = findChild<QLabel*>(labelName);
		lblLines[i]->setText(labelName);
	}
}

OverlayWidget::~OverlayWidget()
{}



void OverlayWidget::SetLine(uint8_t index, QString text) {
	lblLines[index]->setText(text);
}
void OverlayWidget::ClearLine(uint8_t index) {
	lblLines[index]->setText("");
}
uint32_t GetNearestWhitespace(QString text, uint32_t start) {
	if (text.at(start).isSpace()) return start;
	int max = (text.length() > 2 * start) ? start : (text.length() - start);
	for (int i = 1; i < max; i++) {
		if (text.at(start + i).isSpace()) return start + i;
		if (text.at(start - i).isSpace()) return start - i;
	}
	return text.length();
}

void OverlayWidget::SetText(QString text) {
	uint32_t len = text.length();

	uint32_t len2 = len / 2;
	if (len2 > minCharPerLine && len2 < maxCharPerLine) {
		uint32_t split = GetNearestWhitespace(text, len2);
		QString text1 = text.left(split);
		QString text2 = text.right(len - split);
		SetLine(0, text1);
		SetLine(1, text2);
		ClearLine(2);
		return;
	}
	uint32_t len3 = len / 3;
	if (len3 > minCharPerLine) {
		uint32_t split = GetNearestWhitespace(text, len3);
		QString text1 = text.left(split);
		text = text.right(len - split);
		len = len - split;
		split = GetNearestWhitespace(text, len / 2);
		QString text2 = text.left(split);
		QString text3 = text.right(len - split);
		SetLine(0, text1);
		SetLine(1, text2);
		SetLine(2, text3);
		return;
	}

	SetLine(0, text);
	ClearLine(1);
	ClearLine(2);
}
