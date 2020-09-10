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
    #define LOG(x,y) logEvent(x, y)
#else
    #define LOG(x,y)
#endif