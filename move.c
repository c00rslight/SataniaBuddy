#include <X11/Xlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "drag.h"
#include "main.h"
#include "move.h"
#include "animate.h"

enum {move_random, move_window, max_value};

int start_move;

void do_move_random(Display *dpy, Window w)
{
	int newx = rand() % (deskw - window_w);
	int newy = rand() % (deskh - window_h);
	XMoveWindow(dpy, w, newx, newy);

	state = state_sit;
}

void do_move_window(Display *dpy, Window w)
{
	Window root, parent, *children;
	unsigned int n_children;
	XQueryTree(dpy, DefaultRootWindow(dpy), &root, &parent, &children,
			&n_children);

	Window eligible[n_children];
	XWindowAttributes eligible_attrs[n_children];
	int el = 0;

	for (int i = 0; i < n_children; children++, i++) {
		XWindowAttributes xwa;
		XGetWindowAttributes(dpy, *children, &xwa);
		if (xwa.map_state == IsViewable && !xwa.override_redirect
				&& xwa.x + window_w <= deskw &&
				xwa.y-SitY + window_h <= deskh && xwa.x > 0 && xwa.y-SitY > 0) {
			eligible_attrs[el] = xwa;
			eligible[el++] = *children;
		}
	}

	if (el == 0) {
		XFree(children);
		if (MoveRandom) do_move_random(dpy, w);
		return;
	}

	int winn = rand() % el;
	Window move_win = eligible[winn];
	XWindowAttributes move_win_attrs = eligible_attrs[winn];

	XMoveWindow(dpy, w, move_win_attrs.x, move_win_attrs.y-SitY);

	XFree(children);

	state = state_sit_window;
	sit_window = move_win;
}

void move(Display *dpy, Window w)
{
	if (mouse_drag) { start_move = 0; return; }
	else if (!start_move || !Move) return;

	int move_type = rand() % max_value;
	switch (move_type) {
		case move_random: if (MoveRandom) do_move_random(dpy, w); break;
		case move_window: if (MoveWindow) do_move_window(dpy, w); break; 
	} // config file should be able to toggle which of these satania can do

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
