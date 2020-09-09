#include <X11/Xlib.h>

#include "common.h"

Display *X11Display;
Window rootWindow;

int sendKeyEvent(int press, char letter, Window window);

// Get the display for the system.
// TODO: Validate this code against multi-monitor setups.
int X11Initialize()
{
    X11Display = XOpenDisplay(NULL);

    if(X11Display == NULL)
        return FAILED;

    rootWindow = DefaultRootWindow(X11Display);
    
    return OK;
}

// Type the string in the currently focused window.
int typeString(char *string)
{
    Window currentWindow;
    int revert;

    // Get the window that has the input focus.
    XGetInputFocus(X11Display, &currentWindow, &revert);

    for(int i = 0; i < strlen(string); i++)
    {
        // ASCII characters from 0x20 to 0xFF directly map to the corresponding KeySym.
        // ASCII characters below 0x20 are just control characters and should never appear (can be ignored).
        if(string[i] < 0x20 || string[i] > 0x7E)
            continue;
        
        if(sendKeyEvent(TRUE, string[i], currentWindow) == FAILED)
            return FAILED;

        if(sendKeyEvent(FALSE, string[i], currentWindow) == FAILED)
            return FAILED;
        
        return OK;
    }
}

// Send a XKeyEvent to the specified window with the keycode of the corresponding ASCII letter.
int sendKeyEvent(int press, char letter, Window window)
{
    XKeyEvent event;

    event.display = X11Display;
    event.window = window;
    event.root = rootWindow;
    event.subwindow = None;
    event.time = CurrentTime;
    event.x = 1;
    event.y = 1;
    event.x_root = 1;
    event.y_root = 1;
    event.same_screen = TRUE;
    event.keycode = XKeysymToKeycode(X11Display, (KeySym) letter);
    event.state = 0;
    event.type = press ? KeyPress : KeyRelease;

    if(XSendEvent(X11Display, window, TRUE, KeyPressMask, (XEvent *) &event) == 0)
        return FAILED;
    else
        return OK;
}

// Close the display.
void X11Terminate()
{
    XCloseDisplay(X11Display);
    return;
}