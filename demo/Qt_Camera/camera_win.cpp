
#include "camera_win.h"


camera_win :: camera_win()
{
	Ui.setupUi(this);
	Ui.label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	Ui.label->setScaledContents(true);
	Ui.pushButton->setText(tr("save"));
	Ui.pushButton_2->setText(tr("pause"));
	Ui.pushButton_3->setText(tr("quit"));
	Ui.pushButton_4->setText(tr("play"));
	connect(Ui.pushButton,SIGNAL(clicked()),this,SLOT(save_camera()));
	connect(Ui.pushButton_2,SIGNAL(clicked()),this,SLOT(pause_camera()));
	connect(Ui.pushButton_3,SIGNAL(clicked()),this,SLOT(quit_camera()));
	connect(Ui.pushButton_4,SIGNAL(clicked()),this,SLOT(play_camera()));
	this->timer = new QTimer(this);
	connect(this->timer,SIGNAL(timeout()),this,SLOT(read_camera()));
	if(camer_ops :: open_camer() < 0) 
		return ;
//	else
//		timer->start(100);
	camer = new camer_ops;
	connect(camer,SIGNAL(ready(QImage)),this,SLOT(show_picture(QImage)));
	camer->start();
}


camera_win :: ~camera_win(void)
{
	qDebug() << "in ~camera_win()";
}


void camera_win :: play_camera(void)
{
	timer->start(200);
}


void camera_win :: pause_camera(void)
{
	timer->stop();
}


void camera_win :: read_camera(void)
{
	Ui.label->setPixmap(QPixmap::fromImage(camer_ops::read_photo()));
}


void camera_win :: save_camera(void)
{
	camer_ops :: save_photo((QDateTime::currentDateTime()).toString("yyyy_MM_dd_hh_mm_ss") +
							".jpg");
} 

void camera_win :: quit_camera(void)
{
	camer_ops :: close_camer();
	this->close();
}


void camera_win :: show_picture(QImage image)
{
	Ui.label->setPixmap(QPixmap::fromImage(image));
}





