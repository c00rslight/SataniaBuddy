#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * TODO
 *  search for files in other locations, not just the current directory
 *  movement
 *  actions
 */

int window_h;
int window_w;

int MoveIntervalStart;
int MoveIntervalStop;
int Move;

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
	else if (!start_move || !Move) return;

	int newx = rand() % (deskw - window_w);
	int newy = rand() % (deskh - window_h);
	XMoveWindow(dpy, w, newx, newy);

	start_move = 0;
}

void *move_thread(void *p)
{
	for (;;) {
		int sleep_amt = (rand() % (MoveIntervalStop - MoveIntervalStart
					+ 1)) + MoveIntervalStart;
		sleep(sleep_amt);
		start_move = 1;
	}
}

void read_config()
{
	// this should be able to be called from satania's right click menu
	
	// set defaults
	Move = 1;
	MoveIntervalStart = 1;
	MoveIntervalStop  = 2;
	
	FILE *fp = fopen("satania.cfg", "r");
	if (!fp) return;
	char *str = malloc(1024);

	while (fgets(str, 1024, fp) != NULL) {
		char *value = str;
		char* key = strsep(&value, "=");
		if (key == NULL) continue;

		if (strcmp(key, "MoveIntervalStart") == 0)
			MoveIntervalStart = atoi(value);
		else if (strcmp(key, "MoveIntervalStop") == 0)
			MoveIntervalStop = atoi(value);
		else if (strcmp(key, "Move") == 0) {
			if (strcasecmp(value, "true\n") == 0) Move = 1;
			else Move = 0;
		}
	}

	free(str);
	fclose(fp);
}

int main(void)
{
	read_config();

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

	cairo_surface_t *img = cairo_image_surface_create_from_png("satania.png");
	cairo_t *ictx = cairo_create(img);
	double x1, x2, y1, y2;
	cairo_clip_extents(ictx, &x1, &y1, &x2, &y2);
	cairo_destroy(ictx);
	window_w = x2 - x1;
	window_h = y2 - y1;

	Window w = XCreateWindow(dpy, XDefaultRootWindow(dpy), 0, 0,
			window_w, window_h, 0, vinfo.depth, InputOutput, vinfo.visual,
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
			window_w, window_h);
	cairo_xlib_surface_set_size(sfc, window_w, window_h);
	cairo_t *ctx = cairo_create(sfc);
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
