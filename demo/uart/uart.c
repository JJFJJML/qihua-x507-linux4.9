#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h> 
#include <string.h>
#include <sys/ioctl.h>

#define DEV		"/dev/ttyS2"
static int fd = -1;

int serial_init(void)
{
	struct termios options;
    if((fd=open(DEV,O_RDWR|O_NOCTTY|O_NDELAY))==-1) 
    {
    	printf("Open %d device fail\n",DEV);
        return -1;
    }
		
	tcgetattr(fd,&options);
	cfsetispeed(&options,B9600);
	cfsetospeed(&options,B9600);
	options.c_cflag|=(CLOCAL|CREAD); 
	options.c_cflag &= ~PARENB;        
	options.c_iflag &= ~(IGNCR | INLCR | ICRNL | IXON);
	options.c_cflag &= ~CSTOPB;  
	options.c_cflag&=~CSIZE; 
	options.c_cflag|=CS8;    
	options.c_lflag&=~(ICANON|ECHO|ISIG); 
	options.c_oflag&=~OPOST;  

	tcflush(fd,TCIFLUSH);
	tcsetattr(fd,TCSANOW,&options);
	return fd;
}

int serial_write(char *buf, int len)
{
	tcflush(fd, TCIOFLUSH);
	if(write(fd,buf,len) < 0)   
	{
		printf("Write Serial fail\n");
		return -1;
	}
	return 0;
}

int serial_read(char *buff, int len)
{
	if(read(fd,buff,len) < 0)
	{
		printf("Read serial fail\n");
		return -1;
	}
	return 0;
}

int available_len()
{
	int size=0;
	ioctl(fd,FIONREAD,&size);
	return size;
}

int serial_ops(void)
{
	struct timeval tv;  
	int i=0;
	fd_set rdfds; 
	int maxfd = -1;
	int ret;
	char buff[512];
	int len;
	tv.tv_sec = 5;
	tv.tv_usec = 0;     
	printf("Input > ");
	fflush(stdout);
	while(1)
	{
		FD_ZERO(&rdfds);   
		FD_SET(0,&rdfds);
		maxfd = maxfd>0?maxfd:0;
		FD_SET(fd, &rdfds); 
		maxfd = maxfd > fd ? maxfd:fd;
		ret = select(maxfd + 1, &rdfds, NULL, NULL, &tv);
		if(ret < 0)
		{
				perror("Select fail\n");
				return -1;
		}
		else if(ret == 0)
		{
			continue;
		}
		else
		{  
			if(FD_ISSET(fd, &rdfds)) 
			{       
				memset(buff, 0, 512);
				i = 0;
				while((read(fd,&buff[i++],1) > 0)&&(i<511));
				buff[i] = '\0';
				printf("Rev from %s (%s)", DEV , buff );
				printf("\nInput > ");
				fflush(stdout);
			}
			else if(FD_ISSET(0, &rdfds)) 
			{        
				memset(buff, 0, 512);
				fgets(buff,sizeof(buff),stdin);
				buff[511] = '\0';
				printf("Send to %s (%s)", DEV,buff);
				printf("\nInput > ");
				fflush(stdout);
				if(serial_write(buff, sizeof(buff)) < 0)
				{
					return -1;
				}
			}
		}
	}
	return -1;
}

void serial_close(void)
{
	close(fd);
}


int main(void)
{
	serial_init();
	serial_ops();
	serial_close();
	return 0;
}

