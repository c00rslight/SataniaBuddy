#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

/*
 * TODO
 *  opengl (?)
 *  satania
 *  movement
 *  actions
 */

#define WINDOW_W 144
#define WINDOW_H 288

int mouse_drag;
int dragx, dragy;
int deskw, deskh;

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

int main(void)
{
	int run = 1;

	XSetWindowAttributes attr = {0};
	Display *dpy = XOpenDisplay(NULL);
	XVisualInfo vinfo;
	XMatchVisualInfo(dpy, DefaultScreen(dpy), 32, TrueColor, &vinfo);

	attr.colormap =
		XCreateColormap(dpy, DefaultRootWindow(dpy), vinfo.visual, AllocNone);
	attr.override_redirect = True;

	Window w = XCreateWindow(dpy, XDefaultRootWindow(dpy), 0, 0,
			WINDOW_W, WINDOW_H, 0, vinfo.depth, InputOutput, vinfo.visual,
			CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect,
			&attr);

	Screen *screen = ScreenOfDisplay(dpy, DefaultScreen(dpy));
	deskw = screen->width;
	deskh = screen->height;

	XSelectInput(dpy, w,
			ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

	XMapWindow(dpy, w);
	XSync(dpy, w);

	cairo_surface_t *sfc = cairo_xlib_surface_create(dpy, w, vinfo.visual,
			WINDOW_W, WINDOW_H);
	cairo_xlib_surface_set_size(sfc, WINDOW_W, WINDOW_H);
	cairo_t *ctx = cairo_create(sfc);
	cairo_set_source_rgba(ctx, 0, 0, 1, 0.2);
	cairo_paint(ctx);
	
	cairo_destroy(ctx);
	
	while (run) {
		XEvent xe;
		XNextEvent(dpy, &xe);
		switch (xe.type) {
			case ButtonPress: handle_btn_down(&xe); break;
			case ButtonRelease: handle_btn_up(&xe); break;
			case MotionNotify: handle_mousemotion(&xe, dpy, w); break;
		}
	}

	cairo_surface_destroy(sfc);
	XDestroyWindow(dpy, w);
	XCloseDisplay(dpy);
}
