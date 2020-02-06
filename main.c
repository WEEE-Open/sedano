#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>

#define FAILED -1
#define DEV_SERIAL_FILE "/dev/ttyS0"
#define BUFLEN 100

#define FALSE 0
#define TRUE 1

//stty -F /dev/ttyS0 19200 cs8 -cstopb -parenb -crtscts
//stty -F /dev/ttyS0 19200 cs8 -cstopb -parenb -crtscts

void keysend(char *string);
void errorHandler(int, int);
void quit(int);
void debugLoop();
int findStart(char *buf, int);
char *retrieveData(char *buffer, int start, int end);

int scannerSerialDevice;

Display *display;

int isNumber(char *);

int delay = FALSE;
int debug = FALSE;

char buffer[BUFLEN];

int main(int argc, char **argv)
{
	// We need to open the display before we can send any keypress event.
	display = XOpenDisplay(NULL);

	if(!display)
	{
		//complain about the X server failing brutally
		quit(1);
	}

	for(int i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], "--debug"))
		{
			debug = TRUE;
		}
		else if(!strcmp(argv[i], "--delay"))
		{
			if((i+1) < argc && isNumber(argv[i+1]))
			{
				delay = atoi(argv[i+1]);
			}
			else
			{
				//Complain
				quit(1);
			}
		}
	}

	if(debug)
	{
		debugLoop();
	}
	
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
	
	int index = 1;
	
	while(TRUE)
	{
		int count = read(scannerSerialDevice, buffer+index, BUFLEN);
		if(count >= 0)
		{
			index += count;
		}
		else if(count == BUFLEN)
		{
			//complain abou buffer length being exceeded
			quit(1);
		}
		else 
		{
			errorHandler(errno, FALSE);
			quit(1);
		}
		
		if(buffer[index-1] == 3)
		{
			char *result = retrieveData(buffer, findStart(buffer, index-2), index-2);
			index=0;
			memset(&buffer, 0, BUFLEN);
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
				printf("[ERRNO MESSAGE]: %s.\n", strerror(error));
			break;
		case ENOENT:
			printf("ERROR: Device file not found.\n");
			if(verbose)
				printf("[ERRNO MESSAGE]: %s.\n", strerror(error));
			break;
		default:
			printf("ERROR: A general error occurred.\n");
			if(verbose)
				printf("[ERRNO MESSGE]:  %s.\n", strerror(error));
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
			return end-tmp;
		}
		else
		{
			++tmp;
		}
	}

	return -1;
}

char *retrieveData(char *buffer, int start, int end)
{
	char *result = calloc(end-start+2, sizeof(char));
	memset(result, 0, end-start+2);

	sprintf(result, "%.*s", end-start+1, buffer+start);
	return result;
}

void debugLoop()
{
	while(TRUE)
	{

		scanf("%s", buffer);
		keysend(buffer);
	}	
}

void keysend(char *string)
{
	sleep(delay);
	
	for(int i = 0; i < strlen(string); ++i)
	{		
		if(isupper(string[i]) || string[i] == '(' || string[i] == ')')
		{
			XTestFakeKeyEvent(display, XKeysymToKeycode(display, 0xFFE1), TRUE, 0);

			XTestFakeKeyEvent(display, XKeysymToKeycode(display, (KeySym) string[i]), TRUE, 0);
			XTestFakeKeyEvent(display, XKeysymToKeycode(display, (KeySym) string[i]), FALSE, 0);

			XTestFakeKeyEvent(display, XKeysymToKeycode(display, 0xFFE1), FALSE, 0);
		}
		else if(islower(string[i]) || isdigit(string[i]) || string[i] == '-' || string[i] == '.')
		{

			XTestFakeKeyEvent(display, XKeysymToKeycode(display, (KeySym) string[i]), TRUE, 0);
			XTestFakeKeyEvent(display, XKeysymToKeycode(display, (KeySym) string[i]), FALSE, 0);
		}
		
		XFlush(display);
	}
}

int isNumber(char *string)
{
	for(int i = 0; i < strlen(string); i++)
	{
		if(!isdigit(string[i]))
		{
			return FALSE;
		}
	}

	return TRUE;
}
