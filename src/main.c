#include <signal.h>

#include "common.h"
#include "serial.h"
#include "xorg.h"

void handleSignal(int signal);
void quit();

char *deviceFile;
int loopbackMode = FALSE;
int loopbackDelay = 2;

int main(int argc, char **argv)
{
    // Register SIGTERM and SIGINT signals to allow the program to cleanup after itself on exit.
    signal(SIGTERM, handleSignal);
    signal(SIGINT, handleSignal);

    //TODO: Try to call LOG with invalid strings and observe the results.
    //      Try to call initialization routines twice and see if the second time they skip initialization.
    //      Somehow test for dirty initialization handling.
    //      Parse command line options:
    //          - Device file for scanner
    //          - Quiet mode (disables output, default no)
    //          - Debug level (debug, info, warning, error, fatal).
    //          - Loopback mode (accept "scanner" input from stdin)
    //          - Loopback mode delay
    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "--quiet") == 0)
        {
            beQuiet();
        }
        else if(strcmp(argv[i], "--loglevel") == 0)
        {
            if((i + 1) < argc)
            {
                // If the string is an invalid sequence, it will default to the DEBUG log level (int = 0).
                setLogLevel(atoi(argv[i + 1]));
                ++i;
            }
            else
            {
                // Technically we also accept values outside of that range, however they are useless and therefore not documented.
                LOG(LOG_ERROR, "You need to specify an integer in the range %d - %d after the --loglevel argument.", LOG_DEBUG, LOG_FATAL);
            }
        }
        else if(strcmp(argv[i], "--device") == 0)
        {
            if((i + 1) < argc)
            {
                deviceFile = strdup(argv[i + 1]);
                ++i;
            }
            else
            {
                LOG(LOG_ERROR, "You need to specify a path after the --device argument.", LOG_DEBUG, LOG_FATAL);
            }
        }
        else if(strcmp(argv[i], "--loopback") == 0)
        {
            loopbackMode = TRUE;
        }
        else if(strcmp(argv[i], "--delay") == 0)
        {
            if((i + 1) < argc)
            {
                // If the string is an invalid sequence, it will default to a delay of 0.
                setLogLevel(atoi(argv[i + 1]));
                ++i;
            }
            else
            {
                LOG(LOG_ERROR, "You need to specify an integer after the --delay argument.", LOG_DEBUG, LOG_FATAL);
            }
            
        }
        else if(strcmp(argv[i], "--help") == 0)
        {
            //TODO: Actual documentation
            printf("SUCH DOCUMENTATION\nMUCH HELP\nVERY EXPLAIN\n\n");
            quit(0);
        }
        else
        {
            LOG(LOG_ERROR, "Unrecognized command line option \"%s\"", argv[i]);
        }
    }

    LOG(LOG_INFO, "Starting S.E.D.A.N.O...");

    if(X11Initialize() == FAILED)
    {
        LOG(LOG_FATAL, "ERROR: Failed to initialize X11.");
        exit(1);
    }

    while(TRUE)
    {
        char string[1000];
        scanf("%s", string);

        if(typeString(string, 2) == FAILED)
        {
            LOG(LOG_FATAL, "ERROR: Failed to print the string.");
            exit(1);
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

void quit(int level)
{
    // Call cleanup functions
    serialTerminate();
    X11Terminate();

    exit(level);
}
