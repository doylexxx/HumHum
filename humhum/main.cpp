#include "humhum.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Humhum w;
	w.show();
	return a.exec();
}
