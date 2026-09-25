
#include "camer_ops.h"



bool camer_ops :: isopen = false;

camer_ops :: camer_ops(void)
{
}


camer_ops :: ~camer_ops(void)
{
	this->close_camer();
}


bool camer_ops :: open_camer(void)
{
	if(init_v4l2() < 0)
	{
		camer_ops :: isopen = false;
		return false;
	}
	camer_ops :: isopen = true;
	return true;
}

QImage camer_ops :: read_photo(void)
{
	unsigned char bmp[RGB_SIZE] = "";
	if(! camer_ops :: isopen)
		if(!camer_ops :: open_camer())
			return QImage();
	read_bmp(bmp);
	return QImage(bmp, CAMER_WIDTH, CAMER_HEIGHT, QImage::Format_RGB888);
}


void camer_ops :: save_photo(QString name)
{
	if(name == "")
		return;
	if(name.split(".").size() == 1)
		name += ".jpg";
	(camer_ops :: read_photo()).save(name);
}


int camer_ops :: close_camer(void)
{
	exit_v4l2();
	camer_ops :: isopen = false;
	return 0;
}


void camer_ops :: run()
{
	unsigned char bmp[RGB_SIZE] = "";
	if(!camer_ops :: isopen)
		return;
	while(1)
	{
		read_bmp(bmp);
		emit ready(QImage(bmp, CAMER_WIDTH, CAMER_HEIGHT, QImage::Format_RGB888));
	}
}


