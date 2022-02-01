#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

/*
 * TODO
 *  movement
 *  actions
 */

#define WINDOW_W 75
#define WINDOW_H 227
#define MOVE_INTERVAL_START 1
#define MOVE_INTERVAL_STOP 2

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

int start_move;

void move(Display *dpy, Window w)
{
	if (mouse_drag) { start_move = 0; return; }
	else if (!start_move) return;

	int newx = rand() % (deskw - WINDOW_W);
	int newy = rand() % (deskh - WINDOW_H);
	XMoveWindow(dpy, w, newx, newy);

	start_move = 0;
}

void *move_thread(void *p)
{
	for (;;) {
		int sleep_amt = (rand() % (MOVE_INTERVAL_STOP - MOVE_INTERVAL_START
					+ 1)) + MOVE_INTERVAL_START;
		sleep(sleep_amt);
		start_move = 1;
	}
}

int main(void)
{
	int run = 1;

	time_t t;
	srand(time(&t));

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
	cairo_surface_t *img = cairo_image_surface_create_from_png("satania.png");
	cairo_set_source_surface(ctx, img, 0, 0);
	cairo_paint(ctx);
	
	cairo_destroy(ctx); // move to end when we start doing animation
	
	int x11_fd = ConnectionNumber(dpy);
	fd_set in_fds;
	struct timeval tv, before, after, result;
	tv.tv_sec = 0;
	tv.tv_usec = 4000; // needs high fps or we cant keep up with the mouse

	pthread_t thread_id;
	pthread_create(&thread_id, NULL, move_thread, NULL);

	while (run) {
		gettimeofday(&before, NULL);

		XEvent xe;
		FD_ZERO(&in_fds);
		FD_SET(x11_fd, &in_fds);
		int fds = select(x11_fd+1, &in_fds, NULL, NULL, &tv);
		while (XPending(dpy)) {
			XNextEvent(dpy, &xe);
			switch (xe.type) {
				case ButtonPress: handle_btn_down(&xe); break;
				case ButtonRelease: handle_btn_up(&xe); break;
				case MotionNotify: handle_mousemotion(&xe, dpy, w); break;
			}
		}

		move(dpy, w);

		if (fds > 0) {
			gettimeofday(&after, NULL);
			timersub(&after, &before, &result);
			unsigned long sleep_time = tv.tv_usec -
				(1000000 * result.tv_sec + result.tv_usec);
			if (sleep_time <= tv.tv_usec) usleep(sleep_time);
		}
	}

	cairo_surface_destroy(sfc);
	cairo_surface_destroy(img);
	XDestroyWindow(dpy, w);
	XCloseDisplay(dpy);
}
