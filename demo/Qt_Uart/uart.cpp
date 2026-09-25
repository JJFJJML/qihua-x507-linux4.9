#include "uart.h"

uart :: uart()
{
	Ui.setupUi(this);
		uart_t = new uart_thread;
	thread_start();
	connect(Ui.pushButton, SIGNAL(clicked()), this, SLOT(send()));
	connect(uart_t, SIGNAL(receive_data(QString)), this, SLOT(receive_d(QString)));
}

uart :: ~uart()
{
	uart_t->terminate();
}
void uart :: thread_start()
{
	uart_t->start();
}

void uart :: send()
{
	uart_t->send_data();
}

void uart :: receive_d(QString str)
{
	char *c;
	Ui.label_2->setText(str);
	c = str.toLatin1().data();
	if(c[0] == 'q')
	{
		close();
	}
}
