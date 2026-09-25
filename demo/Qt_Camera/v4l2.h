
#ifndef _V4L2_H_
#define _V4L2_H_


#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h> 
#include <memory.h>
#include <sys/mman.h>    
#include <linux/videodev2.h>
#include <assert.h>
#include <pthread.h>



#define CAMER_HEIGHT 480
#define CAMER_WIDTH  640
#define RGB_SIZE	CAMER_WIDTH*CAMER_HEIGHT*3L


int init_v4l2(void);
int start_capturing(void);
int read_bmp(unsigned char *bmp);
int read_yuv420(unsigned char *yuv420) ;
void exit_v4l2(void);
void init_mul(int *p,int arg);


#endif




