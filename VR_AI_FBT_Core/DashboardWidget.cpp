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

	lblRecenter = findChild<QLabel*>("lblRecenter");
	lblRecenter->setVisible(false);

	lblDampen = findChild<QLabel*>("lblDampen");
	sldDampen = findChild<QSlider*>("sldDampen");

	InitConfig();
}

DashboardWidget::~DashboardWidget()
{}



void DashboardWidget::on_btnCalibrateTrackers_clicked() {
	RecalibrateVirtualControllers();
}

void DashboardWidget::on_btnQuit_clicked() {
	QApplication::quit();
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
void DashboardWidget::on_cbxPMButtonRecenter_clicked(bool checked) {
	if (checked) pmFlags |= PlayspaceMoverFlags::DoubleButtonReset;
	else pmFlags &= ~PlayspaceMoverFlags::DoubleButtonReset;
}

void DashboardWidget::on_cbxButtonOcAX_clicked(bool checked) {
	if (checked) inputButtonMask |= ButtonMasks::OculusAX;
	else inputButtonMask &= ~ButtonMasks::OculusAX;
}
void DashboardWidget::on_cbxButtonOcBY_clicked(bool checked) {
	if (checked) inputButtonMask |= ButtonMasks::OculusBY;
	else inputButtonMask &= ~ButtonMasks::OculusBY;
}
void DashboardWidget::on_cbxButtonOcTrig_clicked(bool checked) {
	if (checked) inputButtonMask |= ButtonMasks::OculusTrigger;
	else inputButtonMask &= ~ButtonMasks::OculusTrigger;
}
void DashboardWidget::on_cbxButtonOcAXpm_clicked(bool checked) {
	if (checked) pmButtonMask |= ButtonMasks::OculusAX;
	else pmButtonMask &= ~ButtonMasks::OculusAX;
}
void DashboardWidget::on_cbxButtonOcBYpm_clicked(bool checked) {
	if (checked) pmButtonMask |= ButtonMasks::OculusBY;
	else pmButtonMask &= ~ButtonMasks::OculusBY;
}
void DashboardWidget::on_cbxButtonOcTrigpm_clicked(bool checked) {
	if (checked) pmButtonMask |= ButtonMasks::OculusTrigger;
	else pmButtonMask &= ~ButtonMasks::OculusTrigger;
}

void DashboardWidget::on_cbxTrackerAnkle_clicked(bool checked) {
	if (PoseTrackers[Poses::left_ankle] != checked) {
		if (checked) trackerIDs[Poses::left_ankle] = getTracker(TrackerNames[Poses::left_ankle]);
		else deleteVirtualDevice(trackerIDs[Poses::left_ankle]);
		PoseTrackers[Poses::left_ankle] = checked;
	}
	if (PoseTrackers[Poses::right_ankle] != checked) {
		if (checked) trackerIDs[Poses::right_ankle] = getTracker(TrackerNames[Poses::right_ankle]);
		else deleteVirtualDevice(trackerIDs[Poses::right_ankle]);
		PoseTrackers[Poses::right_ankle] = checked;
	}
}
void DashboardWidget::on_cbxTrackerKnee_clicked(bool checked) {
	if (PoseTrackers[Poses::left_knee] != checked) {
		if(checked) trackerIDs[Poses::left_knee] = getTracker(TrackerNames[Poses::left_knee]);
		else deleteVirtualDevice(trackerIDs[Poses::left_knee]);
		PoseTrackers[Poses::left_knee] = checked;
	}
	if (PoseTrackers[Poses::right_knee] != checked) {
		if (checked) trackerIDs[Poses::right_knee] = getTracker(TrackerNames[Poses::right_knee]);
		else deleteVirtualDevice(trackerIDs[Poses::right_knee]);
		PoseTrackers[Poses::right_knee] = checked;
	}
}
void DashboardWidget::on_cbxTrackerHip_clicked(bool checked) {
	if (PoseTrackers[Poses::right_hip] != checked) {
		if (checked) trackerIDs[Poses::right_hip] = getTracker(TrackerNames[Poses::right_hip]);
		else deleteVirtualDevice(trackerIDs[Poses::right_hip]);
		PoseTrackers[Poses::right_hip] = checked;
	}
}
void DashboardWidget::on_cbxTrackerChest_clicked(bool checked) {
	if (PoseTrackers[Poses::right_shoulder] != checked) {
		if (checked) trackerIDs[Poses::right_shoulder] = getTracker(TrackerNames[Poses::right_shoulder]);
		else deleteVirtualDevice(trackerIDs[Poses::right_shoulder]);
		PoseTrackers[Poses::right_shoulder] = checked;
	}
}
void DashboardWidget::on_cbxTrackerElbow_clicked(bool checked) {
	if (PoseTrackers[Poses::left_elbow] != checked) {
		if (checked) trackerIDs[Poses::left_elbow] = getTracker(TrackerNames[Poses::left_elbow]);
		else deleteVirtualDevice(trackerIDs[Poses::left_elbow]);
		PoseTrackers[Poses::left_elbow] = checked;
	}
	if (PoseTrackers[Poses::right_elbow] != checked) {
		if (checked) trackerIDs[Poses::right_elbow] = getTracker(TrackerNames[Poses::right_elbow]);
		else deleteVirtualDevice(trackerIDs[Poses::right_elbow]);
		PoseTrackers[Poses::right_elbow] = checked;
	}
}

void DashboardWidget::on_cbxUseHipForward_clicked(bool checked) {
	PoseTracker::SetUseHipForward(checked);
}

bool buttonEnabledtmp[16];
void DashboardWidget::on_btnRecenter_clicked() {
	lblRecenter->setVisible(true);

	int n = 0;

	for (int i = 0; i < 16; i++) {
		if (cameras[i].active)
			n++;
	}

	if (n == 0) {
		lblRecenter->setText("No cameras to recenter");
		return;
	}

	recentering = true;

	lblRecenter->setText("Pick a calibrated camera to use to recenter");

	for (int i = 0; i < 16; i++) {
		buttonEnabledtmp[i] = false;
		if(camerasWithManagers[i])
			if (cameraCalibrateButtons[i]->isEnabled()) {
				cameraCalibrateButtons[i]->setEnabled(false);
				buttonEnabledtmp[i] = true;
			}
	}

	
}
void DashboardWidget::DoneRecenter() {
	recentering = false;
	lblRecenter->setVisible(false);

	for (int i = 0; i < 16; i++) {
		if (camerasWithManagers[i])
			cameraCalibrateButtons[i]->setEnabled(buttonEnabledtmp[i]);
	}
	on_btnShowCamera_clicked();

}
void DashboardWidget::on_btnShowCamera_clicked() {
	for (int i = 0; i < 16; i++) {
		if (cameras[i].active) {
			ShowCameraOverlay(i, 30.0f);
		}
	}
}

void DashboardWidget::on_sldDampen_valueChanged(int value) {
	trackerDampening = (float)value / 100.0f;
	QString sldDampenStr = QString::number(value);
	sldDampenStr.append("%");
	lblDampen->setText(sldDampenStr);
}

bool DashboardWidget::SetLabel(QString labelName, QString text) {
	QLabel* label = findChild<QLabel*>(labelName);
	if (label == nullptr) return false;
	label->setText(text);
	return true;
}

void DashboardWidget::InitConfig() {
	QJsonObject pmConfig = config["PlayspaceMover"].toObject();
	QJsonObject pmButtons = pmConfig["buttons"].toObject();
	QJsonObject buttonConfig = config["buttons"].toObject();
	QJsonObject trackerConfig = config["trackers"].toObject();

	//Set buttons used
	findChild<QCheckBox*>("cbxButtonOcAX")->setChecked(buttonConfig["oculusax"].toBool());
	findChild<QCheckBox*>("cbxButtonOcAXpm")->setChecked(pmButtons["oculusax"].toBool());
	findChild<QCheckBox*>("cbxButtonOcBY")->setChecked(buttonConfig["oculusby"].toBool());
	findChild<QCheckBox*>("cbxButtonOcBYpm")->setChecked(pmButtons["oculusby"].toBool());
	findChild<QCheckBox*>("cbxButtonOcTrig")->setChecked(buttonConfig["oculustrigger"].toBool());
	findChild<QCheckBox*>("cbxButtonOcTrigpm")->setChecked(pmButtons["oculustrigger"].toBool());
	findChild<QCheckBox*>("cbxButtonOcBum")->setChecked(buttonConfig["oculusbumper"].toBool());
	findChild<QCheckBox*>("cbxButtonOcBumpm")->setChecked(pmButtons["oculusbumper"].toBool());
	//set pm settings
	findChild<QCheckBox*>("cbxEnablePM")->setChecked(pmConfig["enabled"].toBool());
	findChild<QCheckBox*>("cbxPMButtonRecenter")->setChecked(pmConfig["doubleButtonReset"].toBool());
	//set tracker settings
	findChild<QCheckBox*>("cbxTrackerAnkle")->setChecked(trackerConfig["ankle"].toBool());
	findChild<QCheckBox*>("cbxTrackerKnee")->setChecked(trackerConfig["knee"].toBool());
	findChild<QCheckBox*>("cbxTrackerHip")->setChecked(trackerConfig["hip"].toBool());
	findChild<QCheckBox*>("cbxTrackerChest")->setChecked(trackerConfig["chest"].toBool());
	findChild<QCheckBox*>("cbxTrackerElbow")->setChecked(trackerConfig["elbow"].toBool());
	findChild<QCheckBox*>("cbxUseHipForward")->setChecked(trackerConfig["useHipForward"].toBool(true));
	
	sldDampen->setValue(100 * config["trackerDampening"].toDouble());
	QString sldDampenStr = QString::number(sldDampen->value());
	sldDampenStr.append("%");
	lblDampen->setText(sldDampenStr);
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
		SetCameraState(index, CameraState::Camera_WaitingForCalibration);
		if (recentering) {
			lblRecenter->setText("Waiting for calibration");
			OnRecenter(index);
		}
		else CalibrateCamera(index);
	});

	cameraGrid->addWidget(cameraNameLabels[index], index, 0);
	cameraGrid->addWidget(cameraStateLabels[index], index, 1);
	cameraGrid->addWidget(cameraCalibrateButtons[index], index, 2);

	cameraGrid->minimumSize().setHeight(rowHeight * index);

	camerasWithManagers[index] = true;
}
