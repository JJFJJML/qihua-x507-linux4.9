#include "uart_thread.h"


uart_thread ::uart_thread(void)
{
	struct termios options;
    if((fd = ::open(DEV,O_RDWR|O_NOCTTY|O_NDELAY))==-1) 
    {
    	::printf("Open %s device fail\n",DEV);
    }
	::tcgetattr(fd,&options);
	::cfsetispeed(&options,B115200);
	::cfsetospeed(&options,B115200);
	options.c_cflag|=(CLOCAL|CREAD); 
	options.c_cflag &= ~PARENB;        
	options.c_iflag &= ~(IGNCR | INLCR | ICRNL | IXON);
	options.c_cflag &= ~CSTOPB;  
	options.c_cflag&=~CSIZE; 
	options.c_cflag|=CS8;    
	options.c_lflag&=~(ICANON|ECHO|ISIG); 
	options.c_oflag&=~OPOST;  

	::tcflush(fd,TCIFLUSH);
	::tcsetattr(fd,TCSANOW,&options);
}

uart_thread ::~uart_thread()
{
	::close(fd);
}

void uart_thread :: run()
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
	::fflush(stdout);
	while(1)
	{
		FD_ZERO(&rdfds);   
		FD_SET(0,&rdfds);
		maxfd = maxfd>0?maxfd:0;
		FD_SET(fd, &rdfds); 
		maxfd = maxfd > fd ? maxfd:fd;
		ret = ::select(maxfd + 1, &rdfds, NULL, NULL, &tv);
		if(ret < 0)
		{
				::perror("Select fail\n");
				return ;
		}
		else if(ret == 0)
		{
			continue;
		}
		else
		{  
			if(FD_ISSET(fd, &rdfds)) 
			{		
				::memset(buff, 0, 512);
				i = 0;
				while((::read(fd,&buff[i++],1) > 0)&&(i<511));
				buff[i] = '\0';
				::fflush(stdout);
				QString str = QString::fromLocal8Bit(buff,sizeof(buff));
				emit receive_data(str);
			}
		}
	}
}

void uart_thread :: send_data()
{
 	char p[6] = "hello";
	::tcflush(fd, TCIOFLUSH);
	if(::write(fd,p,5) < 0)   
	{
		::printf("Write Serial fail\n");
		return ;
	}
}


