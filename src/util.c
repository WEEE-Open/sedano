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

#if defined(DEBUG)
    int logLevel = LOG_DEBUG;
#else
    int logLevel = LOG_ERROR;
#endif

int quiet = FALSE;

char * autoFormat(char *, int, ...);

/*
 *  This class itself contains a few points where logs are genereated. Under some circumstances, this chould
 *  result in an infinite recursion. The logic behind this choice is that the logging facilities themselves are
 *  trusted (by extensive testing) to be reliable and not generate any errors by themselselves. By taking this
 *  assumption to be true, any error logs generated within this file can only be caused by code in other regions
 *  of the program that misbehaved. Therefore, unless the offending instruction itself is infinitely repeated,
 *  no recursion should happen whatsoever.
 * 
 *  NOTE: There can be situations where the logging facilities themselves could generate errors, for example
 *        memory allocation if the system is out of usable memory. There however are extreme conditions that
 *        should not happen in a normal execution (and this code is never called in release anyways).
 * 
 *  TODO: Actually implement extensive tests.
 */

void setLogLevel(const int level)
{
    logLevel = level;
    return;
}

void beQuiet()
{
    quiet = TRUE;
    return;
}

int logEvent(const char *fileName, const int lineNumber, const char* function, int severity, char *format, int count, ...)
{
    if(severity < logLevel || quiet)
        return OK;

    char *color = NULL;
    char *type = NULL;

    switch(severity)
    {
        case LOG_DEBUG:
            color = WHITE;
            type = "DBG";
            break;
        case LOG_WARNING:
            color = YELLOW;
            type = "WRN";
            break;
        case LOG_ERROR:
            color = RED;
            type = "ERR";
            break;
        case LOG_FATAL:
            color = MAGENTA;
            type = "FAT";
            break;
        case LOG_INFO:
        default:
            color = CYAN;
            type = "INF";
    }

    char *prologue = NULL;
    char *prologueFormat = "[%s]: %s:%d (%s): ";

    if((prologue = autoFormat(prologueFormat, countFormatIdentifiers(prologueFormat), type, fileName, lineNumber, function)) == NULL)
    {
        LOG(LOG_ERROR, "Failed to construct log prologue message.");
        return FAILED;
    }

    // Print the intestation and message.
    // Passing VAs by reference so we can use NULL as a signal.
    prettyPrint(TRUE, FALSE, color, prologue, NULL, stdout);
    
    va_list args;
    va_start(args, count);

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
            {
                LOG(LOG_ERROR, "Invalid format string provided: \"%s\"", format);
                return FAILED;
            }
            
            // Check that we're not dealing with a literal %.
            if(i != '%')
                argc++;
        }
    }

    return argc;
}

// Automatically allocate the right amount of space to store a formatted string and return the pointer.
// WARNING: When using this function the resulting string must be free'd after use.
char * autoFormat(char* format, int count, ...)
{
    va_list args;
    int stringLength;

    va_start(args, count);

    // Use vsnprintf to write 0 characters to NULL.
    // It has no effect but returns us the strlen of the formatted string
    if((stringLength = vsnprintf(NULL, 0, format, args)) < 0)
    {
        LOG(LOG_ERROR, "Failed to determine formatted string length.");
        return NULL;
    }

    // Need to re-start the VA list to use it again.
    va_end(args);
    va_start(args, count);

    // TODO: Validate return from malloc
    char *string = malloc((stringLength + 1) * sizeof(char));
    int result = vsnprintf(string, (stringLength + 1), format, args);

    va_end(args);

    if(result < 0)
    {
        return NULL;
        LOG(LOG_ERROR, "Failed to format string.");
    }
    else
        return string;

}

// Finds a switch in the command line
int findSwitch(int argc, char **argv, char *name)
{
    for(int i = 1; i < argc; ++i)
        if((strlen(name) == strlen(argv[i])) && !strcmp(name, argv[i]))
            return TRUE;
    
    return FALSE;
}

// Get value of a parameter from the command line
char *getValue(int argc, char **argv, char *name)
{
    for(int i = 0; i < argc; ++i)
        if((strlen(name) == strlen(argv[i])) && !strcmp(name, argv[i]))
        {
            if((i + 1) < argc && argv[i+1][0] != '-')
                return argv[i+1];
            else
                return NULL;
        }
    
    return NULL;        
}

// Wether the input is a natural number and, optionally, within a closed interval
// On success, return the number
// On failure, return -1
int isNatural(char *number, int min, int max)
{
    int length;

    if(!(length = strlen(number)))
        return FAILED;

    for(int i = 0; i < length; ++i)
        if(number[i] < '0' || number[i] > '9')
            return FAILED;
    
    int num = atoi(number);

    if(min != -1 && num < min)
        return FAILED;
    
    if(max != -1 && num > max)
        return FAILED;
    
    return num;
}