#include <X11/Xlib.h>
#include "mouse.h"

int mouse_drag;
int dragx, dragy;

void handle_mousemotion(XEvent *xe, Display *dpy, Window w)
{
	if (!mouse_drag) return;

	// could probably be optimized
	XWindowAttributes xwa;
	XGetWindowAttributes(dpy, w, &xwa);
	int newx = xwa.x - (dragx - xe->xbutton.x);
	int newy = xwa.y - (dragy - xe->xbutton.y);
	XMoveWindow(dpy, w, newx, newy);
}

void handle_btn_down(XEvent *xe)
{
	if (xe->xbutton.button == Button1) {
		mouse_drag = 1;
		dragx = xe->xbutton.x;
		dragy = xe->xbutton.y;
	}
}

void handle_btn_up(XEvent *xe)
{
	if (xe->xbutton.button == Button1) mouse_drag = 0;
}
