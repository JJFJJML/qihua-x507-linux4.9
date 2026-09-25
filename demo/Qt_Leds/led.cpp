#include "led.h"
#include "ui_led.h"
#include <QProcess>

led::led(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::led)
{
    ui->setupUi(this);
}

led::~led()
{
    delete ui;
}


void led::on_pushButton_2_clicked()
{
    ::system("echo 241 >/sys/class/gpio/export;echo out >/sys/class/gpio/gpio241/direction;echo 1 >/sys/class/gpio/gpio241/value");

}

void led::on_pushButton_5_clicked()
{
    ::system("echo 241 >/sys/class/gpio/export;echo out >/sys/class/gpio/gpio241/direction;echo 0 >/sys/class/gpio/gpio241/value");

}

void led::on_pushButton_3_clicked()
{
    ::system("echo 42 >/sys/class/gpio/export;echo out >/sys/class/gpio/gpio42/direction;echo 1 >/sys/class/gpio/gpio42/value");
}

void led::on_pushButton_4_clicked()
{
    ::system("echo 42 >/sys/class/gpio/export;echo out >/sys/class/gpio/gpio42/direction;echo 0 >/sys/class/gpio/gpio42/value");

}
