#pragma once

#define XK_MISCELLANY
#define XK_LATIN1

#include <X11/Xlib.h>
#include <X11/keysymdef.h>

/*
 *      IMPORTANT NOTICE:
 *      =================
 * 
 *      In order to add a terminator:
 * 
 *      1) Add its name (to be passed on the command line) at the END of the terminatorNames array
 *      2) Add the corresponding symbol (defined in /usr/lib/include/X11/keysymdef.h) to the same position of the terminatorSymbols array
 *      3) Update the terminatorCount to the new value
 * 
 *      NOTES
 *      =====
 * 
 *      1) Terminator 0 MUST *ALWAYS* be NONE/XK_VoidSymbol
 */

const int terminatorCount = 4;

const char * terminatorNames[] =
{
    "NONE",
    "ENTER",
    "TABULATION",
    "SPACE"
};

KeySym terminatorSymbols[] =
{
    XK_VoidSymbol,
    XK_Return,
    XK_Tab,
    XK_space
};