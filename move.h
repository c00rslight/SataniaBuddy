#pragma once

#include <X11/Xlib.h>

void *move_thread(void *p);
void move(Display *dpy, Window w);
