#include <fcntl.h>
#include <termios.h>

#include "common.h"

int deviceFD = FAILED;      // UNIX file descriptor for scanner.
FILE *device = NULL;        // STDLIB file for scanner.
struct termios deviceTTY;   // Serial device for scanner.

int initializedFD = FALSE;
int initializedFL = FALSE;

int serialTerminate();

// Preapre and configure the scanner.
int serialInitialize(char *path)
{
    if((deviceFD = open(path, O_RDONLY | O_NONBLOCK)) == FAILED)
        return serialTerminate();
    
    initializedFD = TRUE;

    if((device = fdopen(deviceFD, "r")) == NULL)
        return serialTerminate();

    initializedFL = TRUE;

    memset(&deviceTTY, 0, sizeof deviceTTY);

    // Get serial device configuration.
    if(tcgetattr(deviceFD, &deviceTTY) == FAILED)
        return serialTerminate();

    // Configure connection parameters.
    deviceTTY.c_cflag &= ~(PARENB|CSTOPB|CRTSCTS|IXON|IXOFF|IXANY);
    deviceTTY.c_cflag |= (CS8|CLOCAL);
    deviceTTY.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
	deviceTTY.c_oflag &= ~(OPOST | ONLCR);

    deviceTTY.c_cc[VTIME] = 0;
    deviceTTY.c_cc[VMIN] = 0;

	cfsetispeed(&deviceTTY, B19200);
	cfsetospeed(&deviceTTY, B19200);

    if(tcsetattr(deviceFD, TCSANOW, &deviceTTY) == FAILED)
        return serialTerminate();

    return OK;
}

// Barcodes are sent as ASCII strings by the scanner.
// Strings are delimited by 0x2 at the start and 0x3 at the end.
char * readBarcode()
{
    char *barcode = malloc(sizeof(char));
    char nextChar;

    int length = 0;

    // Check that no errors occurred while allocating.
    if(barcode == NULL)
    {
        serialTerminate();
        free(barcode);
        return NULL;
    }

    // Wait for the start of a new string to come.
    do
    {
        nextChar = fgetc(device);
    }
    while(nextChar != 0x02);

    while(TRUE)
    {
        nextChar = fgetc(device);

        if(nextChar == EOF)
            continue;

        char *tmp = realloc(barcode, (length + 1) * sizeof(char));

        // Check that no errors occurred while reallocating.
        if(tmp == NULL)
        {
            serialTerminate();
            free(barcode);
            return NULL;
        }

        free(barcode);
        barcode = tmp;

        if(nextChar != 0x03)
        {
            barcode[length++] = nextChar;
        }
        else
        {
            barcode[length++] = 0;
            break;
        }
    }

    return barcode;
}

int serialTerminate()
{
    int e = errno;

    // Logical operators are short-circuited: the close operations only complete if the corresponding boolean is true.
    if(initializedFL && fclose(device) == EOF)
        return errno;
    if(initializedFD && close(deviceFD) == FAILED)
        return errno;

    return e;
}