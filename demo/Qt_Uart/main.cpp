#include <QApplication>
#include <QStringList>
#include <QtGui>
#include "uart.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
	QDesktopWidget *desk = QApplication::desktop();
	uart w;
//	w.move((desk->width() - w.width()) / 2, (desk->height() - w.height()) / 2);
	w.show();
    return app.exec();
} 
