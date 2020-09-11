#include <fcntl.h>
#include <termios.h>

#include "common.h"

int deviceFD = FAILED;      // File descriptor for scanner.
FILE *deviceFS = NULL;      // File stream for scanner.
struct termios deviceTTY;   // Serial device for scanner.

int initializedFD = FALSE;
int initializedFS = FALSE;

int serialTerminate();

int serialInitializationDirty = FALSE;
int serialInitializationComplete = FALSE;

// Preapre and configure the scanner.
// TODO: Dirty and completed booleans
int serialInitialize(char *path)
{
    LOG(LOG_DEBUG, "Initializing serial connection...");

    // If the initialization has already been completed, do nothing
    if(serialInitializationComplete)
    {
        LOG(LOG_DEBUG, "Skipping initialization: already complete.");
        LOG(LOG_DEBUG, "If you want to reinitialize serial, close the current connection first.");
        return OK;
    }

    // We clear this value when we complete initialization. This way we know if a previous attempt failed.
    serialInitializationDirty = TRUE;

    if(serialInitializationDirty)
        LOG(LOG_WARNING, "Previous serial initialization attempt did not complete successfully.");

    if((deviceFD = open(path, O_RDONLY | O_NONBLOCK)) == FAILED)
    {
        LOG(LOG_ERROR, "Failed to open file descriptor for device %s.", path);
        return serialTerminate();
    }

    LOG(LOG_INFO, "File descriptor for device %s (readonly, non-blocking) opened successfully.", path);
    initializedFD = TRUE;

    if((deviceFS = fdopen(deviceFD, "r")) == NULL)
    {
        LOG(LOG_ERROR, "Failed to open file stream for device %s.", path);
        return serialTerminate();
    }

    LOG(LOG_INFO, "File stream for device %s opened successfully.", path);
    initializedFS = TRUE;

    memset(&deviceTTY, 0, sizeof deviceTTY);

    // Get serial device configuration.
    if(tcgetattr(deviceFD, &deviceTTY) == FAILED)
    {
        LOG(LOG_ERROR, "Failed to read serial parameters.");
        return serialTerminate();
    }

    LOG(LOG_INFO, "Serial parameters read successfully.");

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
    {
        LOG(LOG_ERROR, "Failed to set serial parameters.");
        return serialTerminate();
    }

    LOG(LOG_INFO, "Serial parameters set successfully.");

    serialInitializationDirty = FALSE;
    serialInitializationComplete = TRUE;
    return OK;
}

// Barcodes are sent as ASCII strings by the scanner.
// Strings are delimited by 0x2 at the start and 0x3 at the end.
// WARNING: Resulting barcode must be free'd after use.
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
        LOG(LOG_ERROR, "Failed to allocate memory for the barcode.");
        return NULL;
    }

    // Wait for the start of a new string to come.
    do
    {
        nextChar = fgetc(deviceFS);
    }
    while(nextChar != 0x02);

    LOG(LOG_DEBUG, "Waiting for a barcode...");
    while(TRUE)
    {
        nextChar = fgetc(deviceFS);

        if(nextChar == EOF)
            continue;

        char *tmp = realloc(barcode, (length + 1) * sizeof(char));

        // Check that no errors occurred while reallocating.
        if(tmp == NULL)
        {
            serialTerminate();
            free(barcode);
        LOG(LOG_ERROR, "Failed to expand memory for the barcode.");
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

    LOG(LOG_DEBUG, "Barcode read successfully: %s", barcode);
    return barcode;
}

int serialTerminate()
{
    LOG(LOG_DEBUG, "Terminating serial connection...");

    int e = errno;

    // Logical operators are short-circuited: the close operations only complete if the corresponding boolean is true.
    if(initializedFS && fclose(deviceFS) == EOF)
    {
        serialInitializationDirty = TRUE;
        LOG(LOG_ERROR, "Failed to close device file stream.");
        return errno;
    }
    if(initializedFD && close(deviceFD) == FAILED)
    {
        serialInitializationDirty = TRUE;
        LOG(LOG_ERROR, "Failed to close device file descriptor.");
        return errno;
    }

     LOG(LOG_INFO, "Closed serial device file stream and descriptor.");

    serialInitializationDirty = FALSE;
    serialInitializationComplete = FALSE;

    return e;
}