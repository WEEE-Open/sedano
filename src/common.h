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

// Logging meta-function
#define LOG(severity, format, ...) logEvent(__FILE__, __LINE__, __func__, severity, format, countFormatIdentifiers(format), ##__VA_ARGS__)
#define SAMESTR(one, two) (strlen(one) == strlen(two) && strcmp(one, two) == 0)