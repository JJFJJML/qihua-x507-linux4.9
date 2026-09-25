#ifndef UART_THREAD_H_
#define UART_THREAD_H_

#include <QtGui>
extern "C" {
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h> 
#include <string.h>
#include <sys/ioctl.h>
}
#include <qthread.h>

#define DEV		"/dev/ttyS5"

class uart_thread : public QThread
{
	Q_OBJECT
public: 
	uart_thread(void);
	~uart_thread();
	void run(void);

signals:
	void receive_data(QString str);
		
public:
	int fd;
public:
	void send_data();


};
#endif
