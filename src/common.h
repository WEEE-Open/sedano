#pragma once

// External includes
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

// Common constants
#define FAILED -1
#define OK 0
#define TRUE 1
#define FALSE 0

#ifdef DEBUG
    //#define LOG(file, line, function, severity, format, ...) logEvent(file, line, function, severity, format, ##__VA_ARGS__)
    #define LOG(severity, format, ...) logEvent(__FILE__, __LINE__, __func__, severity, format, ##__VA_ARGS__)
#else
    #define LOG(severity, format, ...)
#endif