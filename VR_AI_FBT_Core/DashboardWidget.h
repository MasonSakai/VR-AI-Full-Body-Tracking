#pragma once

#include <QWidget>
#include "ui_DashboardWidget.h"
#include <qlabel.h>
#include <qpushbutton.h>
#include <qgridlayout.h>
#include <qstring.h>
#include <string.h>
#include "PlayspaceMover.h"
#include "vrUtil.h"
#include "CameraManager.h"

enum CameraState {
	Camera_Inactive,
	Camera_Active,
	Camera_Calibrating,
	Camera_WaitingForCalibration,
	Camera_NeedsCalibration,
	Camera_Connecting
};

class DashboardWidget : public QWidget
{
	Q_OBJECT

public:
	DashboardWidget(QWidget* parent = nullptr);
	~DashboardWidget();

	void SetCameraState(uint8_t index, CameraState state);
	void ReturnCameraScreenshot(uint8_t index, uint8_t* data[]);

	bool SetLabel(QString label, QString text);

private slots:
	void on_btnRecenter_clicked();
	void on_btnCalibrateTrackers_clicked();
	void on_btnResetPM_clicked();
	void on_cbxEnablePM_clicked(bool checked);
	void on_btnQuit_clicked();

	void on_cbxButtonOcAX_clicked(bool checked);
	void on_cbxButtonOcBY_clicked(bool checked);
	void on_cbxButtonOcTrig_clicked(bool checked);
	void on_cbxButtonOcAXpm_clicked(bool checked);
	void on_cbxButtonOcBYpm_clicked(bool checked);
	void on_cbxButtonOcTrigpm_clicked(bool checked);

private:
	Ui::DashboardWidgetClass ui;

	QGridLayout* cameraGrid;
	bool camerasWithManagers[16];
	QLabel* cameraNameLabels[16];
	QLabel* cameraStateLabels[16];
	QPushButton* cameraCalibrateButtons[16];
	void CreateCameraLabel(uint8_t index);

	void InitTrackerDisplays();
	void InitConfig();
};

