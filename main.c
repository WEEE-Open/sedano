#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>

#define FAILED -1
#define DEV_SERIAL_FILE "/dev/ttyS0"
#define BUFLEN 100

#define FALSE 0
#define TRUE 1

//stty -F /dev/ttyS0 19200 cs8 -cstopb -parenb -crtscts
//stty -F /dev/ttyS0 19200 cs8 -cstopb -parenb -crtscts

void errorHandler(int, int);
void quit(int);
int findStart(char *buf, int);

int scannerSerialDevice;

int main(int argc, char **argv)
{
	char buffer[BUFLEN];
	
	scannerSerialDevice = open(DEV_SERIAL_FILE, O_RDONLY | O_NONBLOCK);

	if(scannerSerialDevice == FAILED)
	{
		errorHandler(errno, FALSE);
		quit(1);
	}

	struct termios serialTTY;
	memset(&serialTTY, 0, sizeof (struct termios));

	if(tcgetattr(scannerSerialDevice, &serialTTY))
	{
		errorHandler(errno, FALSE);
		quit(1);
	}

	serialTTY.c_cflag &= ~PARENB;
	serialTTY.c_cflag &= ~CSTOPB;
	serialTTY.c_cflag &= ~CRTSCTS;
	serialTTY.c_cflag |= CLOCAL;

	serialTTY.c_cflag &= ~(IXON | IXOFF | IXANY);
	serialTTY.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
	serialTTY.c_oflag &= ~(OPOST | ONLCR);

	serialTTY.c_cflag |= CS8;

	serialTTY.c_cc[VTIME] = 0;
	serialTTY.c_cc[VMIN] = 0;

	cfsetispeed(&serialTTY, B19200);
	cfsetospeed(&serialTTY, B19200);

	if(tcsetattr(scannerSerialDevice, TCSANOW, &serialTTY))
	{
		errorHandler(errno, FALSE);
		quit(1);
	}

	memset(&buffer, 0, BUFLEN);
	
	int index = 0;
	
	while(TRUE)
	{
		int count = read(scannerSerialDevice, buffer+index, BUFLEN);
		if(count >= 0)
			index += count;
		else 
		{
			errorHandler(errno, FALSE);
			quit(1);
		}
		
		if(buffer[index-1] == 3)
		{
			int inizio = findStart(buffer, index-1);
			printf("%d", inizio);
			//sprintf(buffer, "%.*s", inizio, buffer+index-inizio);
			printf("%s", buffer);
					
			memset(&buffer, 0, BUFLEN);
			index=0;
		}
	}

	return 0;
}

void errorHandler(int error, int verbose)
{
	switch(error)
	{

		case EACCES:
			printf("ERROR: Permission denied (did you run as root?).\n");
			if(verbose)
				printf("[ERRNO MESSAGE]: %s.", strerror(error));
			break;
		case ENOENT:
			printf("ERROR: File not found. (did you use setserial to find the correct name?).");
			if(verbose)
				printf("[ERRNO MESSAGE]: %s.", strerror(error));
			break;
		default:
			printf("I/O Error while completing operation.\n[ERRNO MESSGE]:  %s.", strerror(errno));
	}
}

void quit(int errorlevel)
{
	close(scannerSerialDevice);
	exit(errorlevel);
}

int findStart(char *data, int end)
{
	int tmp = 1;
	while((end-tmp) >= 0)
	{
		if(data[end-1-tmp] == 2)
		{
			return end-1-tmp;
		}
		else
		{
			++tmp;
		}
	}

	return -1;
}
