#include "common.h"
#include "serial.h"
#include "xorg.h"

int main(int argc, char **argv)
{
    LOG(LOG_INFO, "Sedano started...");

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