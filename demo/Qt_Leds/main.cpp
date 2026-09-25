#include "led.h"
#include <QApplication>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont font =a.font();
    font.setPixelSize(16);
    a.setFont(font);
    led w;
    w.show();
    w.move ((QApplication::desktop()->width() - w.width())/2,(QApplication::desktop()->height() - w.height())/2);


    return a.exec();
}
