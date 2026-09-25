
#ifndef CAMERA_WIN_H_
#define CAMERA_WIN_H_


#include <QtGui>
#include "camer_ops.h"
#include "ui_camera.h"



class camera_win : public QWidget
{
	Q_OBJECT

public:
	camera_win();
	~camera_win(void);
	
public:

private:
	Ui_Form Ui;
	QTimer *timer;
	camer_ops *camer;

public slots:
	void play_camera(void);
	void pause_camera(void);
	void read_camera(void);
	void save_camera(void);
	void quit_camera(void);
	void show_picture(QImage);
	
};





#endif







