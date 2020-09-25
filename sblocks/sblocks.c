#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

/* macros */
#define BLKLEN     256
#define BLKN       LENGTH(blks)
#define CLOCK      CLOCK_MONOTONIC
#define LENGTH(X)  (sizeof X / sizeof X[0])
#define FROMSIG(X) (31 - (X))
#define TOSIG(X)   (31 - (X))
#define SEC        1
#define NSEC       0
#define SIGMAX     31
#define STSLEN     512

typedef struct {
	char *strBefore;
	char *cmd;
	char *strAfter;
	unsigned int period;
	unsigned int sig;
} Blk;

/* function declarations */
static void CloseFifo(void);
static void CmdsToStr(void);
static void OnQuit(int s);
static void OpenDisplay(void);
static void OpenFifo(void);
static void (*Print) (void);
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
static void UpdateFifo(void);

/* include configuration file before variable declerations */
#include "config.h"

/* variables */
static char blkstr[BLKN][BLKLEN];
static char OutStr[STSLEN];
static int fifofd;
static char *fifo = "/tmp/sblocks.fifo";
static int T = -1;
static int Restart = 0;
static int Running = 1;
static int UsingFifo = 0;
static int LastSignal = 0;
static struct timespec *next_ts = &(struct timespec) { 0, 0 };
static struct timespec *curr_ts = &(struct timespec) { 0, 0 };
static struct timespec *sleep_ts = &(struct timespec) { 0, 0 };
/* X11 specific */
static Display *dpy;
static Window root;
static int screen;

/* function implementations */
void
CloseFifo(void)
{
	if (UsingFifo) {
		close(fifofd);
		unlink(fifo);
	}
	UsingFifo = 0;
}

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
OnQuit(int s)
{
	Running = 0;
	if (s == SIGHUP) /* restart */
		Restart = 1;
	if (UsingFifo)
		CloseFifo();
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
OpenFifo(void)
{
	struct stat st;

	if (Print == SetRoot) {
		if (stat(fifo, &st) == 0)
			unlink(fifo);
		if (mkfifo(fifo, 0600) != 0) {
			perror("mkfifo");
			return;
		}
		if ((fifofd = open(fifo, O_WRONLY)) == -1) {
			perror("open");
			return;
		}
	}
	UsingFifo = 1;
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
	UpdateFifo();
}

void
SigHan(int s)
{
	switch (s) {
	case SIGUSR1:
		OpenFifo();
		break;
	case SIGPIPE:
		CloseFifo();
		break;
	default:
		LastSignal = FROMSIG(s);
	}
}

void
SigSetup(void)
{
	int i, s;

	for (i = 16; i < SIGMAX; ++i)
		signal(i, SIG_IGN);

	signal(SIGINT, OnQuit);
	signal(SIGTERM, OnQuit);
	signal(SIGHUP, OnQuit);

	signal(SIGUSR1, SigHan);
	signal(SIGPIPE, SigHan);

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

void
UpdateFifo(void)
{
	if (!UsingFifo)
		return;
	write(fifofd, OutStr, strlen(OutStr));
	write(fifofd, "\n", 1);
}

int
main(int argc, char *argv[])
{
	Print = SetRoot;
	SetOutStr = CmdsToStr;
	if (argc > 1) {
		if (argv[1][0] == '-') {
			switch (argv[1][1]) {
			case 'o':
				Print = StdoutPrint;
				break;
			}
		} else {
			fifo = argv[1];
		}
	}
	if (argc > 2)
		fifo = argv[2];

	SigSetup();
	Run();

	if (Restart)
		execvp(argv[0], argv);
	OnQuit(SIGTERM);

	return EXIT_SUCCESS;
}
