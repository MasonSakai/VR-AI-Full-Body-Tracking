#include "DashboardOverlay.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DashboardOverlay w;
    w.show();
    return a.exec();
}
