#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

/* macros */
#define TEN_9      1000000000
#define SEC        1
#define NSEC       0.0
#define LENGTH(X)  (sizeof X / sizeof X[0])
#define TOSIG(X)   (31 - (X))
#define FROMSIG(X) (31 - (X))
#define SIGMAX     31
#define BLKLEN     256
#define STSLEN     512

typedef struct {
	char *strBefore;
	char *cmd;
	unsigned int period;
	unsigned int sig;
} Blk;

/* function declarations */
static void CleanQuit(int unused);
static void InitBlkstr(void);
static void MainLoop(void);
static void (*Print) (void);
static void SetOutStr(void);
static void SetRoot(void);
static void SetSignals(void);
static void SigHan(int s);
static void Sleep(const struct timespec *rqtp, struct timespec *rem);
static void StdoutPrint(void);
static void UpdateBlk(int i);
static int UpdateCheck(int time);

/* include configuration file before variable declerations */
#include "config.h"

/* variables */
static const int blkn = LENGTH(blks);
static char **blkstr;
static unsigned int FastPrint = 0;
static unsigned int Looping = 1;
static char OutStr[STSLEN];
static const struct timespec *T = &(struct timespec) { SEC, NSEC };
/* X11 specific */
static Display *dpy;
static Window root;
static int screen;

/* function implementations */
void
CleanQuit(int unused)
{
	Looping = 0;
	exit(0);
}

void
InitBlkstr()
{
	int i;

	blkstr = (char **) malloc(blkn * (sizeof (char *)));
	for (i = 0; i < blkn; ++i)
		blkstr[i] = (char *) malloc(BLKLEN);
}

void
MainLoop()
{
	int t = -1;
	struct timespec *rem;

	rem = (struct timespec *) malloc(sizeof(struct timespec *));
	while (Looping) {
		if (UpdateCheck(++t)) {
			SetOutStr();
			Print();
		}
		Sleep(T, rem);
	}
}

void
SetRoot()
{
	Display *d;

	if (d = XOpenDisplay(NULL))
		dpy = d;
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	XStoreName(dpy, root, OutStr);
	XCloseDisplay(dpy);
//	fprintf(stderr, "RootSet completed.\n");
}

void
SetSignals()
{
	int i, s;

	signal(SIGINT, CleanQuit);
	signal(SIGTERM, CleanQuit);
	
	for (i = 1; i < SIGMAX; ++i)
		signal(i, SIG_IGN);

	for (i = 0; i < blkn; ++i) {
		s = TOSIG(blks[i].sig);
		if (s != 0)
			signal(s, SigHan);
	}
}

void
SigHan(int s)
{
//	fprintf(stderr, "SigHandling begins...\t");
	int i, t = FROMSIG(s);

	for (i = 0; i < blkn; ++i) {
		if (blks[i].sig == t)
			UpdateBlk(i);
	}
	SetOutStr();
	Print();
//	fprintf(stderr, "SigHandling completed.\n");
}

void
Sleep(const struct timespec *rqtp, struct timespec *rem)
{
	if (nanosleep(rqtp, rem))
		Sleep(rem, rem);
}

void
StdoutPrint()
{
	printf("%s\n", OutStr);
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
UpdateCheck(int T)
{
	int i, per, U;

	for (i = U = 0; i < blkn; ++i) {
		per = blks[i].period;
		if (per != 0 && T % per == 0 || T == 0) {
			UpdateBlk(i);
			U = 1;
		}
	}
	return U;
}

void
SetOutStr()
{
	int i, e;

	for (i = e = 0; i < blkn; ++i)
		e += sprintf(OutStr + e, "%s%s", blks[i].strBefore, blkstr[i]);
}

int
main(int argc, char *argv[])
{
	if (argc > 1 && !strcmp(argv[1], "-o"))
		Print = StdoutPrint;
	else
		Print = SetRoot;
	InitBlkstr();
	SetSignals();
	MainLoop();
	return EXIT_SUCCESS;
}
