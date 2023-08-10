#include "OverlayWidget.h"

QString labelBase = "lblLine";

const int pxPerChar = 8,
minCharPerLine = 30,
maxCharPerLine = 40,
pxPerLine = 22,
lineHeight = 16,
marginWidth = 10,
marginHeight = 10;
const float mPerPixel = .00075f;

OverlayWidget::OverlayWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	for (int i = 0; i < 3; i++) {
		QString labelName = labelBase;
		labelName.append(std::to_string(i));
		lblLines[i] = findChild<QLabel*>(labelName);
		lblLines[i]->setText(labelName);
		lblLines[i]->move(marginWidth, marginHeight + i * pxPerLine);
	}

	int height = marginHeight * 2 + lineHeight + pxPerLine * 2;
	int width = maxCharPerLine * pxPerChar + marginWidth * 2;
	resize(width, height);
	vr::VROverlay()->SetOverlayWidthInMeters(OverlayHandle, mPerPixel * width);

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
		int height = marginHeight * 2 + lineHeight + pxPerLine;
		int width = len2 * pxPerChar + marginWidth * 2;
		//resize(width, height);
		//vr::VROverlay()->SetOverlayWidthInMeters(OverlayHandle, mPerPixel * width);
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
		int height = marginHeight * 2 + lineHeight + 2 * pxPerLine;
		int width = len3 * pxPerChar + marginWidth * 2;
		//resize(width, height);
		//vr::VROverlay()->SetOverlayWidthInMeters(OverlayHandle, mPerPixel * width);
		return;
	}

	SetLine(0, text);
	ClearLine(1);
	ClearLine(2);

	int height = marginHeight * 2 + lineHeight;
	int width = len * pxPerChar + marginWidth * 2;
	//resize(width, height);
	//vr::VROverlay()->SetOverlayWidthInMeters(OverlayHandle, mPerPixel * width);
}
