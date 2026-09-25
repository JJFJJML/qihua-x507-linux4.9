#ifndef LED_H
#define LED_H

#include <QWidget>
extern "C" {
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
}

namespace Ui {
class led;
}

class led : public QWidget
{
    Q_OBJECT

public:
    explicit led(QWidget *parent = 0);
    ~led();

private slots:

    void on_pushButton_2_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::led *ui;
};

#endif // LED_H
