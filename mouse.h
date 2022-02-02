#pragma once

#include <X11/Xlib.h>

extern int mouse_drag;

void handle_mousemotion(XEvent *xe, Display *dpy, Window w);
void handle_btn_down(XEvent *xe);
void handle_btn_up(XEvent *xe);
