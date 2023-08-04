#include "DashboardWidget.h"

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

	InitInputControls();
	InitTrackerDisplays();
	InitConfig();
}

DashboardWidget::~DashboardWidget()
{}


void DashboardWidget::InitInputControls() {
	connect(findChild<QCheckBox*>("cbxButtonOcAX"), &QCheckBox::clicked, this, [](bool checked) {
		if (checked) inputButtonMask |= ButtonMasks::OculusAX;
		else inputButtonMask &= ~ButtonMasks::OculusAX;
		});
	connect(findChild<QCheckBox*>("cbxButtonOcBY"), &QCheckBox::clicked, this, [](bool checked) {
		if (checked) inputButtonMask |= ButtonMasks::OculusBY;
		else inputButtonMask &= ~ButtonMasks::OculusBY;
		});
	connect(findChild<QCheckBox*>("cbxButtonOcTrig"), &QCheckBox::clicked, this, [](bool checked) {
		if (checked) inputButtonMask |= ButtonMasks::OculusTrigger;
		else inputButtonMask &= ~ButtonMasks::OculusTrigger;
		});
	connect(findChild<QCheckBox*>("cbxButtonOcAXpm"), &QCheckBox::clicked, this, [](bool checked) {
		if (checked) pmButtonMask |= ButtonMasks::OculusAX;
		else pmButtonMask &= ~ButtonMasks::OculusAX;
		});
	connect(findChild<QCheckBox*>("cbxButtonOcBYpm"), &QCheckBox::clicked, this, [](bool checked) {
		if (checked) pmButtonMask |= ButtonMasks::OculusBY;
		else pmButtonMask &= ~ButtonMasks::OculusBY;
		});
	connect(findChild<QCheckBox*>("cbxButtonOcTrigpm"), &QCheckBox::clicked, this, [](bool checked) {
		if (checked) pmButtonMask |= ButtonMasks::OculusTrigger;
		else pmButtonMask &= ~ButtonMasks::OculusTrigger;
		});
}
void DashboardWidget::InitTrackerDisplays() {

}
void DashboardWidget::InitConfig() {

}


void DashboardWidget::on_btnRecenter_clicked() {
	OnRecenter();
}
void DashboardWidget::on_btnCalibrateTrackers_clicked() {
	RecalibrateVirtualControllers();
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

void DashboardWidget::SetCameraState(uint8_t index, CameraState state) {
	if (!camerasWithManagers[index])
		CreateCameraLabel(index);
	switch (state) {
	case CameraState::Camera_Inactive:
		cameraStateLabels[index]->setText("Inactive");
		cameraCalibrateButtons[index]->setEnabled(false);
		cameras[index].active = false;
		break;
	case CameraState::Camera_Active:
		cameraStateLabels[index]->setText("Active");
		cameraCalibrateButtons[index]->setEnabled(true);
		cameras[index].active = true;
		break;
	case CameraState::Camera_Calibrating:
		cameraStateLabels[index]->setText("Calibrating");
		cameraCalibrateButtons[index]->setEnabled(false);
		break;
	case CameraState::Camera_WaitingForCalibration:
		cameraStateLabels[index]->setText("Waiting for Calibration");
		cameraCalibrateButtons[index]->setEnabled(false);
		break;
	case CameraState::Camera_NeedsCalibration:
		cameraStateLabels[index]->setText("Requires Calibration");
		cameraCalibrateButtons[index]->setEnabled(true);
		break;
	case CameraState::Camera_Connecting:
		cameraStateLabels[index]->setText("Initializing");
		cameraCalibrateButtons[index]->setEnabled(false);
		cameras[index].active = false;
		break;
	default:
		cameraStateLabels[index]->setText("Inactive");
		cameraCalibrateButtons[index]->setEnabled(false);
		cameras[index].active = false;
	}
}

void DashboardWidget::CreateCameraLabel(uint8_t index) {
	std::string name = "Camera ";
	name.append(std::to_string(index));
	cameraNameLabels[index] = new QLabel(name.c_str());
	cameraStateLabels[index] = new QLabel("INIT");
	cameraCalibrateButtons[index] = new QPushButton(txtCalibrate);
	connect(cameraCalibrateButtons[index], &QPushButton::clicked, this, [=]() {
		CalibrateCamera(index);
	});

	cameraGrid->addWidget(cameraNameLabels[index], index, 0);
	cameraGrid->addWidget(cameraStateLabels[index], index, 1);
	cameraGrid->addWidget(cameraCalibrateButtons[index], index, 2);

	cameraGrid->minimumSize().setHeight(rowHeight * index);

	camerasWithManagers[index] = true;
}
