#include <X11/Xlib.h>

#include "common.h"

Display *X11Display;
Window rootWindow;

int errorHandler(Display *, XErrorEvent *);
int sendKeyEvent(int press, char letter, Window window);
int X11Terminate();

int X11InitializationComplete = FALSE;
int X11InitializationDirty = FALSE;

// Get the display for the system.
// TODO: Validate this code against multi-monitor setups.
int X11Initialize()
{
    LOG(LOG_INFO, "Initializing X11 interface...");

    // If the initialization has already been completed, do nothing
    if(X11InitializationComplete)
    {
        LOG(LOG_DEBUG, "  Skipping initialization: already complete.");
        LOG(LOG_DEBUG, "  If you want to reinitialize X11, terminate it first.");
        return OK;
    }

    if(X11InitializationDirty)
        LOG(LOG_WARNING, "  Previous X11 initialization attempt did not complete successfully.");

    // We clear this value when we complete initialization. This way we know if a previous attempt failed.
    X11InitializationDirty = TRUE;

    X11Display = XOpenDisplay(NULL);

    if(X11Display == NULL)
    {
        LOG(LOG_ERROR, "  Failed to open display!");
        X11InitializationDirty = TRUE;
        return X11Terminate();
    }

    LOG(LOG_DEBUG, "  Display opened successfully.");

    rootWindow = DefaultRootWindow(X11Display);
    LOG(LOG_DEBUG, "  Hooked to default root window for the display.");

    // We are not interested in the previous handler so we ignore return value.
    XSetErrorHandler(errorHandler);
    LOG(LOG_DEBUG, "  Hooked to X11 error handler.");
    
    LOG(LOG_INFO, "X11 interface initialized!");

    X11InitializationDirty = FALSE;
    return OK;
}

// Type the string in the currently focused window.
int typeString(char *string, int delaySeconds)
{
    // Wait for delay
    if(delaySeconds)
        sleep(delaySeconds);

    // Check wether the passed-in string is NULL to account for possible errors in readBarcode.
    if(string == NULL)
    {
        LOG(LOG_ERROR, "ERROR: The passed-in string is actually NULL!");
        return FAILED;
    }

    LOG(LOG_DEBUG, "Typing the string \"%s\" of length %d into the currently focused window.", string, strlen(string));

    Window currentWindow;
    int revert;

    // Get the window that has the input focus.
    XGetInputFocus(X11Display, &currentWindow, &revert);

    for(int i = 0; i < strlen(string); i++)
    {
        // ASCII characters from 0x20 to 0xFF directly map to the corresponding KeySym.
        // ASCII characters below 0x20 are just control characters and should never appear (can be ignored).
        if(string[i] == '\n')
        {
            // "Silently" ignore newline (probably coming from interactive mode).
            LOG(LOG_DEBUG, "  Ignoring newline in barcode string (probably coming from interactive mode).");
            continue;
        }
        else if(string[i] < 0x20 || string[i] > 0x7E)
        {
            LOG(LOG_WARNING, "  Ignoring invalid ASCII character in input string.");
            continue;
        }

        LOG(LOG_DEBUG, "  Sending keycode 0x%02X corresponding to letter \'%c\'...", XKeysymToKeycode(X11Display, (KeySym) string[i]), string[i]);

        if(sendKeyEvent(TRUE, string[i], currentWindow) == FAILED)
            return FAILED;

        LOG(LOG_DEBUG, "    Sent KeyPress event");

        if(sendKeyEvent(FALSE, string[i], currentWindow) == FAILED)
            return FAILED;

        LOG(LOG_DEBUG, "   Sent KeyRelease event");

        XFlush(X11Display);
    }
        
    return OK;
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

// Handles errors reported by X11.
// Errors can be non-fatal so the function can return but it must not generate
// (in)directly protocol requests on the display that caused the error.
// TODO: Actually do something
int errorHandler(Display *display, XErrorEvent *error)
{
    LOG(LOG_ERROR, "Exception raised by X11 server!");

    // Return value is ignored.
    return OK;
}

// Close the display. 
int X11Terminate()
{
    LOG(LOG_INFO, "Terminating X11 interface...");
    XCloseDisplay(X11Display);
    LOG(LOG_INFO, "Terminated X11 interface!");

    X11InitializationDirty = FALSE;
    X11InitializationComplete = FALSE;

    return OK;
}