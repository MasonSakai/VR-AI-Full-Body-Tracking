#include "DashboardWidget.h"
#include <QtWidgets/QApplication>
#include "websiteServer.h"

int main(int argc, char* argv[])
{

	QApplication a(argc, argv);
	//DashboardWidget w;
	//w.show();
	StartServer();
	return a.exec();
}
