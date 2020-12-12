#include <signal.h>

#include "common.h"
#include "serial.h"
#include "xorg.h"

void handleSignal(int signal);
void parseCommandLine(int argc, char **argv);
int parseTerminator(char * string);
void help(char *path);
void quit();

// "Global" variables with relative defaults
char * deviceFile      = "/dev/ttyS0";    // First serial device on the system. Seems a reasonable default.

int    loopbackMode    = FALSE;           // Don't use stdin by default.
int    loopbackDelay   = 2;               // Two seconds should be just enough to switch windows with ALT+TAB.

int    setSerial       = TRUE;            // Set serial parameters by default.

terminators terminator = NONE;            // What keypress to send to Xorg after the string hs been typed.

int main(int argc, char **argv)
{
    // Register SIGTERM and SIGINT signals to allow the program to cleanup after itself on exit.
    signal(SIGTERM, handleSignal);
    signal(SIGINT, handleSignal);

    //TODO: Try to call LOG with invalid strings and observe the results.
    //      Try to call initialization routines twice and see if the second time they skip initialization.
    parseCommandLine(argc, argv);

    LOG(LOG_INFO, "Starting S.E.D.A.N.O...");

    if(X11Initialize() == FAILED)
    {
        LOG(LOG_FATAL, "ERROR: Failed to initialize X11.");
        exit(1);
    }

    if(loopbackMode)
    {
        printf("Insert a series of strings that will be treated as if read from the scanner (max 255 characters).\n");
        while (TRUE)
        {
            char buffer[256];

            printf(">>> ");
            fgets(buffer, 256, stdin);

            if(typeString(buffer, loopbackDelay, terminator) == FAILED)
            {
                LOG(LOG_FATAL, "ERROR: Failed to print the string.");
                exit(1);
            }
        }
    }
    else
    {
        if(serialInitialize(deviceFile, setSerial) != OK)
        {
            LOG(LOG_FATAL, "ERROR: Failed to open serial connection to device.");
            exit(1);
        }

        while(TRUE)
        {
            char *string = readBarcode();

            if(typeString(string, 0, terminator) == FAILED)
            {
                LOG(LOG_FATAL, "ERROR: Failed to print the string.");
                exit(1);
            }

            // This string has been malloc'd
            free(string);
        }
    }
}

void handleSignal(int signal)
{
    switch(signal)
    {
        case SIGTERM:
        case SIGINT:
            LOG(LOG_INFO, "Cleaning up before exit...");
            quit(0);
    }
}

void parseCommandLine(int argc, char **argv)
{
    // Boolean switches
    if(FINDSWITCH("--help") || FINDSWITCH("-h"))
        help(argv[0]);

    if(FINDSWITCH("--quiet"))
        beQuiet();

    setSerial = !FINDSWITCH("--nosetserial");
    loopbackMode = FINDSWITCH("--loopback");

    // Strings
    char *device = GETVALUE("--device");

    if(device != NULL)
        deviceFile = device;

    // Ints
    char *delay = GETVALUE("--delay");
    char *loglevel = GETVALUE("--loglevel");

    int parsedDelay = (delay) ? isNatural(delay, -1, -1) : -1;
    int parsedLevel = (loglevel) ? isNatural(loglevel, LOG_DEBUG, LOG_FATAL) : -1;

    if(parsedDelay != -1)
        loopbackDelay = parsedDelay;
    
    if(parsedLevel != -1)
        setLogLevel(parsedLevel);

    char *terminator = GETVALUE("--terminator");

    if(!parseTerminator(terminator))
        LOG(LOG_ERROR, "\"%s\" is not a valid terminator: disabling terminator.");
}

int parseTerminator(char * string)
{
    if(string == NULL)
        return 1;

    if(SAMESTR(string, "ENTER"))
        terminator = ENTER;
    else if(SAMESTR(string, "TABULATION"))
        terminator = TABULATION;
    else if(SAMESTR(string, "SPACE"))
        terminator = SPACE;
    else
    {
        terminator = NONE;
        return 0;
    }

    return 1;
}

void help(char *path)
{
    //TODO: Actual documentation
    printf("Usage: %s [options]\n", path);
    printf("\nCommand line options:\n");
    printf("    --device <path>    : Specifies device file to use.\n\n");
    printf("    --terminator <key> : Terminates all inputs with a given keypress. See the following section for valid terminators.\n");
    printf("    --loglevel <level> : Specifies output loglevel (%d = Debug, %d = Fatal).\n", LOG_DEBUG, LOG_FATAL);
    printf("    --delay <seconds>  : Specifies seconds of delay between scanner read and X11 write.\n\n");
    printf("    --loopback         : Enables loopback mode (read from stdin instead of scanner).\n");
    printf("    --nosetserial      : Skips serial parameter initialization.\n\n");
    printf("    --quiet            : Suppresses ALL output (including fatal errors).\n");
    printf("    --help             : Shows this screen.\n");
    printf("\nValid terminator IDs:\n");
    printf("    ENTER\n");
    printf("    TABULATION\n");
    printf("    SPACE\n");
    exit(0);
}

void quit(int level)
{
    // Call cleanup functions
    serialTerminate();
    X11Terminate();

    exit(level);
}
