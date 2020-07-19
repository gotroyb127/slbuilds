#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

/* macros */
#define CLOCK      CLOCK_MONOTONIC
#define SEC        1
#define NSEC       0
#define LENGTH(X)  (sizeof X / sizeof X[0])
#define TOSIG(X)   (31 - (X))
#define FROMSIG(X) (31 - (X))
#define SIGMAX     31
#define BLKLEN     256
#define STSLEN     512
#define BLKN       LENGTH(blks)

typedef struct {
	char *strBefore;
	char *cmd;
	char *strAfter;
	unsigned int period;
	unsigned int sig;
} Blk;

/* function declarations */
static void (*Print) (void);
static void Quit(int s);
static void Run(void);
static void SetOutStr(void);
static void SetRoot(void);
static void SigHan(int s);
static void SigSetup(void);
static void Sleep();
static void StdoutPrint(void);
static void ts_diff(const struct timespec *A, const struct timespec *B, struct timespec *t);
static int UpdateAll(int t);
static void UpdateBlk(int i);

/* include configuration file before variable declerations */
#include "config.h"

/* variables */
static char blkstr[BLKN][BLKLEN];
static int LastSignal = 0;
static char OutStr[STSLEN];
static int Restart = 0;
static int Running = 1;
static struct timespec *next_ts = &(struct timespec) {};
static struct timespec *curr_ts = &(struct timespec) {};
static struct timespec *sleep_ts = &(struct timespec) {};
/* X11 specific */
static Display *dpy;
static Window root;
static int screen;

/* function implementations */
void
Quit(int s)
{
	Running = 0;
	if (s == SIGHUP)
		Restart = 1;
}

void
Run(void)
{
	int t = -1;

	while (Running) {
		clock_gettime(CLOCK, curr_ts);
		*next_ts = (struct timespec) { curr_ts->tv_sec + SEC,
		                           curr_ts->tv_nsec + NSEC };

		if (UpdateAll(++t)) {
			SetOutStr();
			Print();
		}
		Sleep();
	}
}

void
SetOutStr(void)
{
	int i, e;

	for (i = e = 0; i < BLKN; ++i) {
		e += sprintf(OutStr + e, "%s%s%s",
		     blks[i].strBefore, blkstr[i], blks[i].strAfter);
	}
}

void
SetRoot(void)
{
	Display *d;

	if (d = XOpenDisplay(NULL))
		dpy = d;
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	XStoreName(dpy, root, OutStr);
	XCloseDisplay(dpy);
}

void
SigHan(int s)
{
	LastSignal = FROMSIG(s);
}

void
SigSetup(void)
{
	int i, s;

	for (i = 1; i < SIGMAX; ++i)
		signal(i, SIG_IGN);

	signal(SIGINT, Quit);
	signal(SIGTERM, Quit);
	signal(SIGHUP, Quit);

	for (i = 0; i < BLKN; ++i) {
		s = TOSIG(blks[i].sig);
		if (s != 0)
			signal(s, SigHan);
	}
}

void
Sleep()
{
	if (!Running)
		return;
	if (LastSignal) {
		for (int i = 0; i < BLKN; ++i) {
			if (blks[i].sig == LastSignal)
				UpdateBlk(i);
		}
		LastSignal = 0;
		SetOutStr();
		Print();
	}
	clock_gettime(CLOCK, curr_ts);
	ts_diff(next_ts, curr_ts, sleep_ts);
	if (nanosleep(sleep_ts, NULL))
		Sleep();
}

void
StdoutPrint(void)
{
	printf("%s\n", OutStr);
}

void
ts_diff(const struct timespec *A, const struct timespec *B, struct timespec *t)
{
	*t = (struct timespec) { A->tv_sec - B->tv_sec, A->tv_nsec - B->tv_nsec };
	if (t->tv_nsec < 0) {
		t->tv_nsec += 1e9;
		--t->tv_sec;
	}
	if (t->tv_sec < 0) {
		t->tv_sec = 0;
		t->tv_nsec = 1e3;
	}
}

int
UpdateAll(int t)
{
	int i, per, U;

	for (i = U = 0; i < BLKN; ++i) {
		per = blks[i].period;
		if (per != 0 && t % per == 0 || t == 0) {
			UpdateBlk(i);
			U = 1;
		}
	}
	return U;
}

void
UpdateBlk(int i)
{
	FILE *cmdout;
	char * s;

	cmdout = popen(blks[i].cmd, "r");
	if (!fgets(blkstr[i], BLKLEN, cmdout))
		*blkstr[i] = '\0';
	pclose(cmdout);
}

int
main(int argc, char *argv[])
{
	if (argc > 1 && !strcmp(argv[1], "-o"))
		Print = StdoutPrint;
	else
		Print = SetRoot;
	SigSetup();
	Run();
	if (Restart) execvp(argv[0], argv);
	return EXIT_SUCCESS;
}
