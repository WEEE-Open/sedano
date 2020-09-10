# pragma once

// Define the log levels

#define LOG_DEBUG   0
#define LOG_INFO    1
#define LOG_WARNING 2
#define LOG_ERROR   3
#define LOG_FATAL   4

void logEvent(int, char*);
void prettyPrint(int, int, char *, char *, FILE *);