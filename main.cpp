#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setOrganizationName("Vadim Balakhanov");
	a.setOrganizationDomain("baldis.org");
	a.setApplicationName("TrackDB");
	MainWindow w;
	w.show();

	return a.exec();
}
