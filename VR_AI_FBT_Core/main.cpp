#include "DashboardWidget.h"
#include <QtWidgets/QApplication>
#include "websiteServer.h"
#include <QDebug>


int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	//DashboardWidget w;
	//w.show();
	qDebug() << "Test";

	AIRemoteServer::SharedInstance()->StartServer();
	return a.exec();
}
