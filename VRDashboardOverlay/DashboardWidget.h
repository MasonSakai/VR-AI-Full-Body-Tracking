#pragma once

#include <QtWidgets/QWidget>
#include "ui_DashboardWidget.h"
#include "DashboardListener.h"
#include <qlabel.h>
#include <qpushbutton.h>
#include <qgridlayout.h>

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    DashboardWidget(IDashboardListener* dashboardListener, QWidget *parent = nullptr);
    ~DashboardWidget();

    void SetCameraState(uint8_t index, uint8_t state);
    void ReturnCameraScreenshot(uint8_t index, uint8_t* data[]);


private slots:
    void on_btnRecenter_clicked();
    void on_btnCalibrateTrakers_clicked();
    void on_btnResetPM_clicked();
    void on_cbxEnablePM_clicked(bool checked);
    void on_btnQuit_clicked();

private:
    Ui::DashboardWidgetClass ui;
    IDashboardListener* listener;

    QGridLayout* cameraGrid;
    bool camerasWithManagers[16];
    QLabel* cameraNameLabels[16];
    QLabel* cameraStateLabels[16];
    QPushButton* cameraCalibrateButtons[16];
    void CreateCameraLabel(uint8_t index);

};
