#pragma once

#include <X11/Xlib.h>

extern int state;
extern Window sit_window;
enum {state_sit, state_sit_window};

void animate(Display *dpy, Window w);
