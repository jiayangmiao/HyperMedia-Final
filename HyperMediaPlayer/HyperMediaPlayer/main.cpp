#include "HyperMediaEntry.h"
#include "HyperMediaPlayer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	HyperMediaEntry w;
	HyperMediaPlayer player(9000,352,288,30);
	player.show();
	//w.show();
	return a.exec();
}
