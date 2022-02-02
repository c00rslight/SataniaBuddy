#include <X11/Xlib.h>
#include "animate.h"
#include "main.h"
#include "drag.h"

int state;
Window sit_window;

void goto_window(Display *dpy, Window w)
{
	if (mouse_drag) { state = state_sit; return; }
	XWindowAttributes xwa;
	XGetWindowAttributes(dpy, sit_window, &xwa);
	XMoveWindow(dpy, w, xwa.x, xwa.y-SitY);
}

void animate(Display *dpy, Window w)
{
	switch (state) {
		case state_sit_window: goto_window(dpy, w); break;
	}
}
