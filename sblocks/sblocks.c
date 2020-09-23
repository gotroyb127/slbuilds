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
static void CmdsToStr(void);
static void EchoRootName(void);
static void OpenDisplay(void);
static void (*Print) (void);
static void Quit(int s);
static void Run(void);
static void (*SetOutStr) (void);
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
static char OutStr[STSLEN];
static int LastSignal = 0;
static int Restart = 0;
static int Running = 1;
static int T = -1;
static struct timespec *next_ts = &(struct timespec) { 0, 0 };
static struct timespec *curr_ts = &(struct timespec) { 0, 0 };
static struct timespec *sleep_ts = &(struct timespec) { 0, 0 };
/* X11 specific */
static Display *dpy;
static Window root;
static int screen;

/* function implementations */
void
CmdsToStr(void)
{
	int i, e;

	if (!UpdateAll(T))
		return;

	for (i = e = 0; i < BLKN; ++i) {
		e += sprintf(OutStr + e, "%s%s%s",
		     blks[i].strBefore, blkstr[i], blks[i].strAfter);
	}
}

void
EchoRootName(void)
{
	char *name;

	/* sleep so that xrootname gets updated */
	if (LastSignal)
		nanosleep(&(struct timespec) { 0, 1e8 }, NULL);

	OpenDisplay();
	XFetchName(dpy, root, &name);
	XCloseDisplay(dpy);

	strcpy(OutStr, name);
}

void
OpenDisplay(void)
{
	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "sblocks: cannot open display\n");
		exit(1);
	}

	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
}

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
	while (Running) {
		T += 1;
		clock_gettime(CLOCK, curr_ts);
		*next_ts = (struct timespec) { curr_ts->tv_sec + SEC,
		                               curr_ts->tv_nsec + NSEC };
		SetOutStr();
		Print();
		Sleep();
	}
}

void
SetRoot(void)
{
	OpenDisplay();
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
		SetOutStr();
		Print();
		LastSignal = 0;
	}
	clock_gettime(CLOCK, curr_ts);
	ts_diff(next_ts, curr_ts, sleep_ts);
/*	fprintf(stderr, "%ld.%09ld\n", sleep_ts->tv_sec, sleep_ts->tv_nsec); */
	if (nanosleep(sleep_ts, NULL))
		Sleep();
}

void
StdoutPrint(void)
{
	write(1, OutStr, strlen(OutStr));
	write(1, "\n", 1);
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
		if ((per != 0 && t % per == 0) || t == 0) {
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

	cmdout = popen(blks[i].cmd, "r");
	if (!fgets(blkstr[i], BLKLEN, cmdout))
		*blkstr[i] = '\0';
	pclose(cmdout);
}

int
main(int argc, char *argv[])
{
	Print = SetRoot;
	SetOutStr = CmdsToStr;
	if (argc > 1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 'p':
			SetOutStr = EchoRootName;
		/* fallthrough */
		case 'o':
			Print = StdoutPrint;
			break;
		}
	}

	SigSetup();
	Run();

	if (Restart)
		execvp(argv[0], argv);
	return EXIT_SUCCESS;
}
