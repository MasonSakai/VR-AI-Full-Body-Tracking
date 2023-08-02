#include "DashboardWidget.h"
#include <qstring.h>
#include <string.h>

QString txtCalibrate("Calibrate");

const int rowHeight = 48,
btnHeight = 24;

DashboardWidget::DashboardWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	for (int i = 0; i < 16; i++) camerasWithManagers[i] = false;
	cameraGrid = findChild<QGridLayout*>("gridCameras");
	cameraGrid->minimumSize().setHeight(rowHeight);
}

DashboardWidget::~DashboardWidget()
{}


void DashboardWidget::on_btnRecenter_clicked() {

}
void DashboardWidget::on_btnCalibrateTrakers_clicked() {

}
void DashboardWidget::on_btnResetPM_clicked() {
	pmOffset = glm::vec3();
	UpdateHardwareOffset();
}
void DashboardWidget::on_cbxEnablePM_clicked(bool checked) {
	pmOffset = glm::vec3();
	if (checked) {
		if (!(pmFlags & PlayspaceMoverFlags::Active)) {
			pmFlags |= PlayspaceMoverFlags::Active;
			EnableHardwareOffset();
		}
	}
	else
	{
		if (pmFlags & PlayspaceMoverFlags::Active) {
			pmFlags &= ~PlayspaceMoverFlags::Active;
			DisableHardwareOffset();
		}
	}
}
void DashboardWidget::on_btnQuit_clicked() {
	QApplication::quit();
}

void DashboardWidget::SetCameraState(uint8_t index, uint8_t state) {
	if (!camerasWithManagers[index])
		CreateCameraLabel(index);
	if (state & 1) {
		cameraStateLabels[index]->setText("Active");
		cameraCalibrateButtons[index]->setEnabled(true);
	}
	else if (state & 2) {
		cameraStateLabels[index]->setText("Calibrating");
		cameraCalibrateButtons[index]->setEnabled(false);
	}
	else {
		cameraStateLabels[index]->setText("Inactive");
		cameraCalibrateButtons[index]->setEnabled(false);
	}
}

void DashboardWidget::CreateCameraLabel(uint8_t index) {
	std::string name = "Camera ";
	name.append(std::to_string(index));
	cameraNameLabels[index] = new QLabel(name.c_str());
	cameraStateLabels[index] = new QLabel("INIT");
	cameraCalibrateButtons[index] = new QPushButton(txtCalibrate);
	connect(cameraCalibrateButtons[index], &QPushButton::clicked, this, [=]() {
		//OnCalibrateCamera
	});

	cameraGrid->addWidget(cameraNameLabels[index], index, 0);
	cameraGrid->addWidget(cameraStateLabels[index], index, 1);
	cameraGrid->addWidget(cameraCalibrateButtons[index], index, 2);

	cameraGrid->minimumSize().setHeight(rowHeight * index);

	camerasWithManagers[index] = true;
}
