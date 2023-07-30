#pragma once

#include <QtWidgets/QWidget>
#include "ui_DashboardOverlay.h"

class DashboardOverlay : public QWidget
{
    Q_OBJECT

public:
    DashboardOverlay(QWidget *parent = nullptr);
    ~DashboardOverlay();

private:
    Ui::DashboardOverlayClass ui;
};
