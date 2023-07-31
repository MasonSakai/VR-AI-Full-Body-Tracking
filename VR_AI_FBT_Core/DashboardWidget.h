#pragma once

#include <QtWidgets/QWidget>
#include "ui_DashboardWidget.h"

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    DashboardWidget(QWidget *parent = nullptr);
    ~DashboardWidget();

private:
    Ui::DashboardWidgetClass ui;
};
