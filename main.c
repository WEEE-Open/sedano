#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>

#define FAILED -1
#define DEV_SERIAL_FILE "/dev/ttyS0"

int main(int argc, char **argv)
{
	int scannerSerialDevice = open(DEV_SERIAL_FILE, O_RDONLY | O_NONBLOCK);

	if(scannerSerialDevice == FAILED)
	{
		switch(errno)
		{
			case EACCES:
				printf("Error opening file %s: Permission denied (did you run as sudo?).", DEV_SERIAL_FILE);
				break;
			case ENOENT:
				printf("Device file not found. Try to use setserial to get the correct name.");
				break;
			default:
				printf("I/O Error while opening file: %s.", strerror(errno));

		}

		exit(1);
	}

	
	
}
