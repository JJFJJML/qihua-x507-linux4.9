#ifndef _UART_H_
#define _UART_H_

#include <QtGui>
#include "uart_thread.h"
#include "ui_uart.h"

class uart : public QWidget 
{
	Q_OBJECT

public:
	uart();
	~uart();

private:
	Ui_Form Ui;
	uart_thread *uart_t;
	
private slots:
	void send();
	void receive_d(QString str);
	void thread_start();
};

#endif