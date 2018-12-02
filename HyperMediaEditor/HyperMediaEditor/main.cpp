#include "HyperMediaEditor.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	HyperMediaEditor w;
	w.show();
	return a.exec();
}
