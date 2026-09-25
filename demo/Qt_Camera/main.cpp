#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QHBoxLayout>
#include <QIcon>
//#include <QWSServer>
#include "camera_win.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));  
//	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK")); 
//	QTextCodec::setCodecForTr(QTextCodec::codecForName("GBK"));

	QDesktopWidget *desk = QApplication::desktop();
	camera_win w;
	w.show();

    return a.exec();
}




