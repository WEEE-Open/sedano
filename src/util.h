# pragma once

#include <stdarg.h>

// Define the log levels
#define LOG_DEBUG   0
#define LOG_INFO    1
#define LOG_WARNING 2
#define LOG_ERROR   3
#define LOG_FATAL   4

void setLogLevel(const int level);
void beQuiet();
int logEvent(const char *, const int, const char*, int, char *, int, ...);
void prettyPrint(int, int, char *, char *, va_list *, FILE *);
int countFormatIdentifiers(char *);