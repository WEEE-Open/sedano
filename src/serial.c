#include <fcntl.h>
#include <termios.h>

#include "common.h"

int deviceFD = FAILED;      // File descriptor for scanner.
FILE *deviceFS = NULL;      // File stream for scanner.
struct termios deviceTTY;   // Serial device for scanner.

int initializedFD = FALSE;
int initializedFS = FALSE;

int serialTerminate();
void dumpSerialParameters(struct termios *device);

int serialInitializationDirty = FALSE;
int serialInitializationComplete = FALSE;

// Preapre and configure the scanner.
// TODO: How many of the errno "decorated" functions actually set errno upon a fail?
int serialInitialize(char *path, int setSerial)
{
    LOG(LOG_INFO, "Initializing serial connection...");

    // If the initialization has already been completed, do nothing
    if(serialInitializationComplete)
    {
        LOG(LOG_DEBUG, "  Skipping initialization: already complete.");
        LOG(LOG_DEBUG, "  If you want to reinitialize serial, terminate it first.");
        return OK;
    }

    if(serialInitializationDirty)
        LOG(LOG_WARNING, "  Previous serial initialization attempt did not complete successfully.");

    // We clear this value when we complete initialization. This way we know if a previous attempt failed.
    serialInitializationDirty = TRUE;

    if((deviceFD = open(path, O_RDONLY)) == FAILED)
    {
        LOG(LOG_ERROR, "  Failed to open file descriptor for device %s.", path);
        LOG(LOG_ERROR, "      The error was: %s", strerror(errno));
        return serialTerminate();
    }

    LOG(LOG_DEBUG, "  File descriptor for device %s opened successfully.", path);
    initializedFD = TRUE;

    if((deviceFS = fdopen(deviceFD, "r")) == NULL)
    {
        LOG(LOG_ERROR, "  Failed to open file stream for device %s.\n    The error was: %s\n", path, strerror(errno));
        return serialTerminate();
    }

    LOG(LOG_DEBUG, "  File stream for device %s opened successfully.", path);
    initializedFS = TRUE;

    memset(&deviceTTY, 0, sizeof deviceTTY);

    // Get serial device configuration.
    if(tcgetattr(deviceFD, &deviceTTY) == FAILED)
    {
        LOG(LOG_ERROR, "  Failed to read serial parameters.\n    The error was: %s\n", strerror(errno));
        return serialTerminate();
    }

    LOG(LOG_DEBUG, "  Serial parameters read successfully.");

    dumpSerialParameters(&deviceTTY);

    if(setSerial)
    {
        // Configure connection parameters.
        deviceTTY.c_cflag &= ~(ICANON|PARENB|CSTOPB|CRTSCTS|IXON|IXOFF|IXANY);
        deviceTTY.c_cflag |=  (CS8|CLOCAL);

        deviceTTY.c_iflag &= ~(ISIG|ECHO|ICANON|IEXTEN|IGNBRK|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
        deviceTTY.c_iflag |=  (BRKINT|IGNPAR);

	    deviceTTY.c_oflag &= ~(ISIG|ECHO|ICANON|IEXTEN|OPOST|ONLCR);

        deviceTTY.c_lflag &= ~(ISIG|ECHO|ICANON|IEXTEN);

        // Wait for one character at least
        deviceTTY.c_cc[VTIME] = 0;
        deviceTTY.c_cc[VMIN] = 1;

	    cfsetispeed(&deviceTTY, B19200);
	    cfsetospeed(&deviceTTY, B19200);

        if(tcsetattr(deviceFD, TCSANOW, &deviceTTY) == FAILED)
        {
            LOG(LOG_ERROR, "  Failed to set serial parameters.\n    The error was: %s\n", strerror(errno));
            return serialTerminate();
        }

        LOG(LOG_DEBUG, "  Serial parameters set successfully.");
    }
    else
        LOG(LOG_INFO, "  Skipping setting serial parameters...");

    dumpSerialParameters(&deviceTTY);

    serialInitializationDirty = FALSE;
    serialInitializationComplete = TRUE;

    LOG(LOG_INFO, "Serial connection initialized!");
    return OK;
}

// Barcodes are sent as ASCII strings by the scanner.
// Strings are delimited by 0x2 at the start and 0x3 at the end.
// WARNING: Resulting barcode must be free'd after use.
char * readBarcode()
{
    LOG(LOG_INFO, "Preparing to read a barcode...");

    char *barcode = malloc(sizeof(char));
    char nextChar;

    int length = 0;

    // Check that no errors occurred while allocating.
    if(barcode == NULL)
    {
        serialTerminate();
        free(barcode);
        LOG(LOG_ERROR, "  Failed to allocate memory for the barcode.");
        return NULL;
    }

    // Wait for the start of a new string to come.
    do
    {
        nextChar = fgetc(deviceFS);
    }
    while(nextChar != 0x02);

    LOG(LOG_DEBUG, "  Waiting for a barcode...");
    while(TRUE)
    {
        nextChar = fgetc(deviceFS);

        if(nextChar == EOF)
            continue;

        barcode = realloc(barcode, (length + 1) * sizeof(char));

        // Check that no errors occurred while reallocating.
        if(barcode == NULL)
        {
            serialTerminate();
            free(barcode);

            LOG(LOG_ERROR, "  Failed to expand memory for the barcode.");
            return NULL;
        }

        if(nextChar != 0x03)
            barcode[length++] = nextChar;
        else
        {
            barcode[length++] = 0;
            break;
        }
    }

    LOG(LOG_DEBUG, "Barcode read successfully: %s", barcode);
    return barcode;
}

// TODO: Are we sure close and fclose set errno?
int serialTerminate()
{
    LOG(LOG_INFO, "Terminating serial connection...");

    int e = errno;

    // Logical operators are short-circuited: the close operations only complete if the corresponding boolean is true.
    // Aparently, closing the filestream also closes the file descriptor.
    if(initializedFS && fclose(deviceFS) == EOF)
    {
        serialInitializationDirty = TRUE;
        LOG(LOG_ERROR, "  Failed to close device file stream and descriptor.");
        return errno;
    }

     LOG(LOG_DEBUG, "  Closed serial device file stream and descriptor.");
     LOG(LOG_INFO, "Serial connection terminated!");

    serialInitializationDirty = FALSE;
    serialInitializationComplete = FALSE;

    return e;
}

// Dump all parameters of the serial connection to screen.
// Mainly used to find correct parameters.
void dumpSerialParameters(struct termios *device)
{
    LOG(LOG_DEBUG, "The device is currently configured as follows:");

    // Input parameters
    LOG(LOG_DEBUG, "  INPUT PARAMETERS (0x%08X):", device->c_iflag);
    LOG(LOG_DEBUG, "    IGNBRK  : %s", (device->c_iflag & IGNBRK ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    BRKINT  : %s", (device->c_iflag & BRKINT ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    IGNPAR  : %s", (device->c_iflag & IGNPAR ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    PARMRK  : %s", (device->c_iflag & PARMRK ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    INPCK   : %s", (device->c_iflag & INPCK  ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    ISTRIP  : %s", (device->c_iflag & ISTRIP ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    INLCR   : %s", (device->c_iflag & INLCR  ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    IGNCR   : %s", (device->c_iflag & IGNCR  ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    ICRNL   : %s", (device->c_iflag & ICRNL  ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    IUCLC   : %s", (device->c_iflag & IUCLC  ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    IXON    : %s", (device->c_iflag & IXON   ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    IXANY   : %s", (device->c_iflag & IXANY  ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    IXOFF   : %s", (device->c_iflag & IXOFF  ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    IMAXBEL : %s", (device->c_iflag & IMAXBEL) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    IUTF8   : %s", (device->c_iflag & IUTF8  ) ? "Yes" : "No");

    // Connection parameters
    LOG(LOG_DEBUG, "  CONNECTION PARAMETERS (0x%08X):", device->c_cflag);
    LOG(LOG_DEBUG, "    CBAUD   : %s", (device->c_cflag & CBAUD  ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    CBAUDEX : %s", (device->c_cflag & CBAUDEX) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    CSIZE   : %s", (device->c_cflag & CSIZE  ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    CSTOPB  : %s", (device->c_cflag & CSTOPB ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    CREAD   : %s", (device->c_cflag & CREAD  ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    PARENB  : %s", (device->c_cflag & PARENB ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    PARODD  : %s", (device->c_cflag & PARODD ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    HUPCL   : %s", (device->c_cflag & HUPCL  ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    CLOCAL  : %s", (device->c_cflag & CLOCAL ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    CIBAUD  : %s", (device->c_cflag & CIBAUD ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    CMSPAR  : %s", (device->c_cflag & CMSPAR ) ? "Yes" : "No");
    LOG(LOG_DEBUG, "    CRTSCTS : %s", (device->c_cflag & CRTSCTS) ? "Yes" : "No");

    // Print abbreviated form of other fields that are less important:
    LOG(LOG_DEBUG, "  LINE PARAMETERS (0x%08X):", device->c_lflag);
    LOG(LOG_DEBUG, "  OUTPUT PARAMETERS (0x%08X):", device->c_oflag);

    // Print timing information
    LOG(LOG_DEBUG, "    VTIME: %d", device->c_cc[VTIME]);
    LOG(LOG_DEBUG, "    VMIN: %d", device->c_cc[VMIN]);
}