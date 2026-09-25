#include "v4l2.h"


static unsigned int n_buffers = 0;
static int tfn[256],ee[256],oet[256],fff[256];
static int fd;
static struct buffer 
{ 
	void* start; 
	unsigned int length; 
}*buffers; 



int init_v4l2(void) 
{
    fd = open ("/dev/video4", O_RDWR | O_NONBLOCK, 0); 
    if (fd == -1)
	{ 
		printf("Can't open video dev\n");
        return -1; 
    }
    struct v4l2_capability cap;
    int ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if(-1 == ret) 
	{
    	printf("Can't get the information\n");
    	goto err_1;
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{ 
        printf("This is not a video capture device\n");
        goto err_1;
    }   
    if(!(cap.capabilities & V4L2_CAP_STREAMING)) 
	{ 
        printf("This is not I / O flow control\n");
        goto err_1;
    } 

	struct v4l2_cropcap cropcap; 
	struct v4l2_crop crop;
    memset(&cropcap,0,sizeof(cropcap)); 
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl (fd, VIDIOC_CROPCAP, &cropcap) == 0) 
	{ 
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
        crop.c = cropcap.defrect;
		ioctl(fd, VIDIOC_S_CROP, &crop);
	} 
	else 
	{
		goto err_1;
	}

	struct v4l2_format fmt;
	memset(&fmt,0,sizeof(fmt));
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    fmt.fmt.pix.width       = CAMER_WIDTH;  
    fmt.fmt.pix.height      = CAMER_HEIGHT; 
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;//V4L2_PIX_FMT_YUV422P; 
    fmt.fmt.pix.field       = V4L2_FIELD_ANY;//V4L2_FIELD_INTERLACED; 
    if (ioctl (fd, VIDIOC_S_FMT, &fmt) == -1) 
	{
    	printf("the second step setting the format is failar\n");
		goto err_1;
	}

	struct v4l2_requestbuffers req; 
	req.count=4; 
	req.type=V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	req.memory=V4L2_MEMORY_MMAP; 
	ret = ioctl(fd,VIDIOC_REQBUFS,&req);
	if(-1 == ret) 
	{
		printf("Application buffer failed\n");
		goto err_1;
	}

	buffers = calloc(req.count, sizeof (*buffers));
	if (NULL == buffers) 
	{ 
        printf ("Out of memory\nAllocate memory for buffers error\n"); 
        goto err_1; 
    }
    for(n_buffers = 0; n_buffers < req.count; ++n_buffers) 
	{ 
        struct v4l2_buffer buf; 
        memset(&buf,0,sizeof(buf)); 
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
        buf.memory      = V4L2_MEMORY_MMAP; 
        buf.index       = n_buffers;
        ioctl (fd, VIDIOC_QUERYBUF, &buf);
        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = mmap (NULL,
					                    buf.length, 
					                    PROT_READ | PROT_WRITE, 
					                    MAP_SHARED, 
					                    fd, 
					                    buf.m.offset);
		if (buffers[n_buffers].start == MAP_FAILED) 
		{
			printf("Mapping the %d buffer queue error\n",n_buffers);
			goto err_2;
		} 
	}


	init_mul(tfn,359);
	init_mul(ee,88);
	init_mul(oet,183);
	init_mul(fff,454);

	if(-1 == start_capturing()) 
	{
		printf("Failed to start the image acquisition\n");
		goto err_2;
	}

	return fd;

err_2:
	free(buffers);
	printf("buffers's free\n");
err_1:
	close(fd);
	return -1;
}


int start_capturing(void) 
{ 
	unsigned int i; 
	enum v4l2_buf_type type; 

	for (i = 0; i < n_buffers; ++i) 
	{ 
		struct v4l2_buffer buf; 
		memset(&buf,0,sizeof(buf)); 
 
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
		buf.memory      = V4L2_MEMORY_MMAP; 
		buf.index       = i; 
		ioctl (fd, VIDIOC_QBUF, &buf);
	} 
                
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	if (-1 == ioctl (fd, VIDIOC_STREAMON, &type)) 
	{
		printf("Video display fails\n");
		return -1;
	}
	return fd; 
} 



void init_mul(int *p,int arg) 
{
	int i;
	for(i = -128; i < 128; i++) 
	{
		p[128+i] = i*arg;
	}
}



int read_yuv420(unsigned char *yuv420) 
{
	struct v4l2_buffer buf; 
	fd_set fds; 
	struct timeval tv; 
	int r; 
	FD_ZERO (&fds); 
	FD_SET (fd, &fds); 
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	r = select (fd + 1, &fds, NULL, NULL, &tv);
	if(r<0) 
	{
		printf("Read yuv420 error!\n");
		goto err;
	} 
	else if(r == 0) 
	{
		printf("Read yuv420 from v4l2 time out!\n");
		goto err;
	} 
	else 
	{
		memset(&buf,0,sizeof(buf));
		unsigned char * line_buffer = (unsigned char *)calloc (CAMER_WIDTH * 3, 1);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
		buf.memory = V4L2_MEMORY_MMAP; 
		if(-1 == ioctl (fd, VIDIOC_DQBUF, &buf)) 
		{
			printf("reading picture error\n");
			goto err;
		}            

		assert (buf.index < n_buffers); 
		unsigned char *yuv;
		yuv = buffers[buf.index].start;

		int i,j;
		unsigned char *pY = yuv420;
		unsigned char *pU = yuv420 + CAMER_WIDTH*CAMER_HEIGHT;
		unsigned char *pV = pU + (CAMER_WIDTH*CAMER_HEIGHT)/4;

		unsigned char *pYUVTemp = yuv;
		unsigned char *pYUVTempNext = yuv+CAMER_WIDTH*2;
        
		for(i=0; i<CAMER_HEIGHT; i+=2)
		{
			for(j=0; j<CAMER_WIDTH; j+=2)
			{
				pY[j] = *pYUVTemp++;
				pY[j+CAMER_WIDTH] = *pYUVTempNext++;
				                
				pU[j>>1] =(*(pYUVTemp) + *(pYUVTempNext))/2;
				pYUVTemp++;
				pYUVTempNext++;
				                
				pY[j+1] = *pYUVTemp++;
				pY[j+1+CAMER_WIDTH] = *pYUVTempNext++;
				                
				pV[j>>1] =(*(pYUVTemp) + *(pYUVTempNext))/2;
				pYUVTemp++;
				pYUVTempNext++;
			}
			pYUVTemp+=CAMER_WIDTH<<1;
			pYUVTempNext+=CAMER_WIDTH<<1;
			pY+=CAMER_WIDTH<<1;
			pU+=CAMER_WIDTH>>1;
			pV+=CAMER_WIDTH>>1;
		}

		free(line_buffer);
		if (-1 == ioctl (fd, VIDIOC_QBUF, &buf)) 
		{
			printf("Failed to sample the picture!!\n");
			goto err;
		}
	}
	return (CAMER_WIDTH*CAMER_HEIGHT*3)>>1;
err:
    return -1;
}


int read_bmp(unsigned char *bmp) 
{
	struct v4l2_buffer buf; 
	fd_set fds; 
	struct timeval tv; 
	int r; 
	FD_ZERO (&fds); 
	FD_SET (fd, &fds); 
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	r = select (fd + 1, &fds, NULL, NULL, &tv);
	if(r<0) 
	{
		printf("Read bmp error!\n");
		goto err;
	} 
	else if(r == 0) 
	{
		printf("Read bmp from v4l2 time out!\n");
		goto err;
	} 
	else 
	{
		memset(&buf,0,sizeof(buf));
		unsigned char * line_buffer = (unsigned char *)calloc (CAMER_WIDTH * 3, 1);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
		buf.memory = V4L2_MEMORY_MMAP; 
		if(-1 == ioctl (fd, VIDIOC_DQBUF, &buf)) 
		{
			printf("reading picture error\n");
			goto err;
		}            

		assert (buf.index < n_buffers); 
		unsigned char *yuv;
		yuv = buffers[buf.index].start;

	    int z = 0;
	    int x;
	  	unsigned char *ptr;
        int r, g, b;
        int y;
        int i,offset = 0,row_stride = CAMER_WIDTH*3;
		for(i = 0; i < CAMER_HEIGHT; i++) 
		{
			ptr = line_buffer;
			for (x = 0; x < CAMER_WIDTH; x++) 
			{
				if (!z)
					y = yuv[0] << 8;
				else
					y = yuv[2] << 8;

				b = (y + (tfn[yuv[3]])) >> 8;
				g = (y - (ee[yuv[1]]) - (oet[yuv[3]])) >> 8;
				r = (y + (fff[yuv[1]])) >> 8;
				*(ptr++) = (b > 255) ? 255 : ((b < 0) ? 0 : b);
				*(ptr++) = (g > 255) ? 255 : ((g < 0) ? 0 : g);
				*(ptr++) = (r > 255) ? 255 : ((r < 0) ? 0 : r);

				if (z++) {
					z = 0;
					yuv += 4;
				}
			}
			offset = row_stride*i;
			memcpy(bmp+offset,line_buffer,row_stride);
		}

		free(line_buffer);
		if (-1 == ioctl (fd, VIDIOC_QBUF, &buf)) 
		{
			printf("Failed to sample the picture!!\n");
			goto err;
		}
	}

	return 0;

err:
 
    return -1;
}

void exit_v4l2(void)
{	
	int i;
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;	
	i = ioctl(fd, VIDIOC_STREAMOFF, &type);
	if (0 > i) 
	{
		perror("Close-came");
		exit(-1);
	}

	for(i = 0; i < (int)n_buffers; i++) 
	{
		munmap(buffers[i].start,buffers[i].length);
	}
	free(buffers);
	n_buffers = 0;
	close(fd);
}







