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

#include "main.h"
#include "drag.h"
#include "move.h"
#include "animate.h"

int deskw, deskh;

int window_h;
int window_w;

int MoveIntervalStart;
int MoveIntervalStop;
int Move;
int SitY;
int MoveRandom;
int MoveWindow;

/*
 * TODO
 *  search for files in other locations, not just the current directory
 *  movement
 *  actions
 */

void read_config()
{
	// this should be able to be called from satania's right click menu
	
	// set defaults
	Move = 1;
	MoveIntervalStart = 1;
	MoveIntervalStop  = 2;
	SitY = 148;
	MoveRandom = 1;
	MoveWindow = 1;
	
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
		else if (strcmp(key, "SitY") == 0)
			SitY = atoi(value);
		else if (strcmp(key, "MoveRandom") == 0) {
			if (strcasecmp(value, "true\n") == 0) MoveRandom = 1;
			else MoveRandom = 0;
		}
		else if (strcmp(key, "MoveWindow") == 0) {
			if (strcasecmp(value, "true\n") == 0) MoveWindow = 1;
			else MoveWindow = 0;
		}
	}

	free(str);
	fclose(fp);
}

int error_handler(Display *dpy, XErrorEvent *e)
{
	if (e->error_code == 3) {
		state = state_sit;
		return 0;
	}
	printf("X Error: %d\n", e->error_code);
	return 1;
}

int main(void)
{
	read_config();

	int run = 1;

	time_t t;
	srand(time(&t));

	XSetErrorHandler(error_handler);

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
		animate(dpy, w);

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
