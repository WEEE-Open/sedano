#include <signal.h>

#include "common.h"
#include "serial.h"
#include "xorg.h"

void handleSIGINT();
void handleSIGTERM();
void quit();

int main(int argc, char **argv)
{
    LOG(LOG_INFO, "Starting S.E.D.A.N.O...");

    // Register SIGTERM and SIGINT signals to allow the program to cleanup after itself on exit.
    signal(SIGTERM, handleSIGTERM);
    signal(SIGINT, handleSIGINT);


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

void handleSIGTERM()
{
    LOG(LOG_WARNING, "Received SIGTERM!");
    LOG(LOG_INFO, "Cleaning up before exit...");
    quit(0);
}

void handleSIGINT()
{
    LOG(LOG_WARNING, "Received SIGINT!");
    LOG(LOG_INFO, "Cleaning up before exit...");
    quit(0);
}

void quit(int level)
{
    // Call cleanup functions
    serialTerminate();
    X11Terminate();

    exit(level);
}