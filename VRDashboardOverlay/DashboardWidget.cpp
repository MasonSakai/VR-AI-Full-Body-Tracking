#include "DashboardWidget.h"
#include <qstring.h>
#include <string.h>

QString txtCalibrate("Calibrate");

const int rowHeight = 48,
		  btnHeight = 24;

DashboardWidget::DashboardWidget(IDashboardListener* dashboardListener, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	listener = dashboardListener;
	for (int i = 0; i < 16; i++) camerasWithManagers[i] = false;
	cameraGrid = findChild<QGridLayout*>("gridCameras");
	cameraGrid->minimumSize().setHeight(rowHeight);
}

DashboardWidget::~DashboardWidget()
{}


void DashboardWidget::on_btnRecenter_clicked() {
	listener->OnRecenter();
}
void DashboardWidget::on_btnCalibrateTrakers_clicked() {
	listener->OnCalibrateTrackers();
}
void DashboardWidget::on_btnResetPM_clicked() {
	listener->OnResetPM();
}
void DashboardWidget::on_cbxEnablePM_clicked(bool checked) {
	listener->OnSetPM(checked);
}
void DashboardWidget::on_btnQuit_clicked() {
	listener->QuitProgram();
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
		listener->OnCalibrateCamera(index);
	});

	cameraGrid->addWidget(cameraNameLabels[index], index, 0);
	cameraGrid->addWidget(cameraStateLabels[index], index, 1);
	cameraGrid->addWidget(cameraCalibrateButtons[index], index, 2);

	cameraGrid->minimumSize().setHeight(rowHeight * index);

	camerasWithManagers[index] = true;
}
