#include "DashboardWidget.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DashboardWidget w;
    w.show();
    return a.exec();
}
