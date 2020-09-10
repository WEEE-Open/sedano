#include "common.h"
#include "util.h"

// Console formatting codes for foreground color
#define RED         "\x1B[31m"
#define GREEN       "\x1B[32m"
#define YELLOW      "\x1B[33m"
#define BLUE        "\x1B[34m"
#define MAGENTA     "\x1B[35m"
#define CYAN        "\x1B[36m"
#define WHITE       "\x1B[37m"

// Console formatting codes for style
#define BOLD        "\x1B[1m"
#define UNDERLINE   "\x1B[4m"

// Reset formatting
#define RESET       "\x1B[0m"

void logEvent(int severity, char *message)
{
    char *color = NULL;
    char *intestation;

    switch(severity)
    {
        case LOG_DEBUG:
            color = WHITE;
            intestation = "[DEBUG]: ";
            break;
        case LOG_WARNING:
            color = YELLOW;
            intestation = "[WARN]: ";
            break;
        case LOG_ERROR:
            color = RED;
            intestation = "[ERROR]: ";
            break;
        case LOG_FATAL:
            color = MAGENTA;
            intestation = "[FATAL]: ";
            break;
        case LOG_INFO:
        default:
            color = CYAN;
            intestation = "[INFO]: ";
    }

    // Print the intestation and message
    // TODO: Does the bold actually work?
    prettyPrint(TRUE, FALSE, color, intestation, stdout);
    prettyPrint(FALSE, TRUE, color, message, stdout);

    return;
}

void prettyPrint(int bold, int newline, char *color, char *message, FILE *stream)
{
    if(bold)
        fprintf(stream, BOLD);

    if(color != NULL)
        fprintf(stream, color);

    fprintf(stream, "%s%s", message, RESET);

    if(newline)
        fprintf(stream, "\n");
    
    return;
}