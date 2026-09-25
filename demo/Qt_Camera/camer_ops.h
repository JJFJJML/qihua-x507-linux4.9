
#ifndef _CAMER_OPS_H_
#define _CAMER_OPS_H_


#include <QtGui>
extern "C" {
#include "v4l2.h"
}



class camer_ops  : public QThread
{
	Q_OBJECT
		
public:
	camer_ops(void);
	~camer_ops(void);

public:
	static bool open_camer(void);
	static QImage read_photo(void);
	static void save_photo(QString);
	static int close_camer(void);
	void run();
	
public:

	static bool isopen;

signals:
	void ready(QImage);

};


#endif



