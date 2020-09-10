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

int countFormatIdentifiers(char *);
int autoFormat(char **, char *, ...);

int logEvent(const char *fileName, const int lineNumber, const char* function, int severity, char *format, ...)
{
    char *color = NULL;
    char *type = NULL;

    switch(severity)
    {
        case LOG_DEBUG:
            color = WHITE;
            type = "DEBUG";
            break;
        case LOG_WARNING:
            color = YELLOW;
            type = "WARN";
            break;
        case LOG_ERROR:
            color = RED;
            type = "ERROR";
            break;
        case LOG_FATAL:
            color = MAGENTA;
            type = "FATAL";
            break;
        case LOG_INFO:
        default:
            color = CYAN;
            type = "INFO";
    }

    char * prologue = NULL;
    if(autoFormat(&prologue, "[%s]: %s:%d (%s): ", type, fileName, lineNumber, function) < 0)
        return FAILED;

    // Print the intestation and message.
    // Passing VAs by reference so we can use NULL as a signal.
    prettyPrint(TRUE, FALSE, color, prologue, NULL, stdout);
    
    va_list args;
    va_start(args, countFormatIdentifiers(format));

    prettyPrint(FALSE, TRUE, color, format, &args, stdout);

    va_end(args);
    free(prologue);
    return OK;
}

void prettyPrint(int bold, int newline, char *color, char *format, va_list *arguments, FILE *stream)
{
    if(bold)
        fprintf(stream, BOLD);

    if(color != NULL)
        fprintf(stream, color);

    if(arguments != NULL)
        vfprintf(stream, format, *arguments);
    else
        fprintf(stream, format);
    
    fprintf(stream, RESET);

    if(newline)
        fprintf(stream, "\n");
    
    return;
}

// Determine the number of format identifiers (excluding %%).
// Corresponds of the number of arguments in the VA list of a printf-like function.
int countFormatIdentifiers(char *format)
{
    int argc = 0;
    for(int i = 0; i < strlen(format); i++)
    {
        if(format[i] == '%')
        {
            // We need to only evaluate the next character.
            i++;

            // Malformed string: a % must be followed by at least one char.
            if(i >= strlen(format))
                return FAILED;
            
            // Our character is a placeholder and not a literal %.
            if(i != '%')
                argc++;
        }
    }

    return argc;
}

// Automatically allocate the right amount of space to store a formatted string and return the pointer.
// WARNING: When using this function the resulting string must be free'd after use.
int autoFormat(char **string, char* format, ...)
{
    va_list args;
    va_start(args, countFormatIdentifiers(format));

    // Use vsnprintf to write 0 characters to NULL.
    // It has no effect but returns us the strlen of the formatted string
    int stringLength = vsnprintf(NULL, 0, format, args);
    if(stringLength < 0)
        return FAILED;

    // Need to re-start the VA list to use it again.
    va_end(args);
    va_start(args, countFormatIdentifiers(format));

    *string = malloc((stringLength + 1) * sizeof(char));
    
    int result = vsnprintf(*string, (stringLength + 1), format, args);

    va_end(args);

    if(result < 0)
        return FAILED;
    else
        return OK;

}