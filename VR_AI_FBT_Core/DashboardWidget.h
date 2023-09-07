#pragma once

#include <QWidget>
#include "ui_DashboardWidget.h"
#include <qlabel.h>
#include <qpushbutton.h>
#include <qgridlayout.h>
#include <qstring.h>
#include <qslider.h>
#include <string.h>
#include "PlayspaceMover.h"
#include "vrUtil.h"
#include "CameraManager.h"
#include "OverlayManager.h"
#include "Config.h"

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

	bool SetLabel(QString label, QString text);

	void DoneRecenter();

private slots:
	void on_btnCalibrateTrackers_clicked();

	void on_btnQuit_clicked();

	void on_btnResetPM_clicked();
	void on_cbxEnablePM_clicked(bool checked);
	void on_cbxPMButtonRecenter_clicked(bool checked);

	void on_cbxButtonOcAX_clicked(bool checked);
	void on_cbxButtonOcBY_clicked(bool checked);
	void on_cbxButtonOcTrig_clicked(bool checked);
	void on_cbxButtonOcAXpm_clicked(bool checked);
	void on_cbxButtonOcBYpm_clicked(bool checked);
	void on_cbxButtonOcTrigpm_clicked(bool checked);

	void on_cbxTrackerAnkle_clicked(bool checked);
	void on_cbxTrackerKnee_clicked(bool checked);
	void on_cbxTrackerHip_clicked(bool checked);
	void on_cbxTrackerChest_clicked(bool checked);
	void on_cbxTrackerElbow_clicked(bool checked);

	void on_btnRecenter_clicked();
	void on_btnShowCamera_clicked();

	void on_sldDampen_valueChanged(int value);

	void on_cbxUseHipForward_clicked(bool checked);

private:
	Ui::DashboardWidgetClass ui;

	QGridLayout* cameraGrid;
	bool camerasWithManagers[16];
	QLabel* cameraNameLabels[16];
	QLabel* cameraStateLabels[16];
	QPushButton* cameraCalibrateButtons[16];
	QLabel* lblRecenter, * lblDampen;
	QSlider* sldDampen;
	void CreateCameraLabel(uint8_t index);

	bool recentering = false;

	void InitConfig();
};

