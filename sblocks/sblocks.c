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
static void Sleep(const struct timespec *rqtp, struct timespec *rem);
static void StdoutPrint(void);
static void UpdateBlk(int i);
static int UpdateCheck(int time);

/* include configuration file before variable declerations */
#include "config.h"

/* variables */
static char blkstr[BLKN][BLKLEN];
static unsigned int Running = 1;
static unsigned int Restart = 0;
static char OutStr[STSLEN];
static const struct timespec *T = &(struct timespec) { SEC, NSEC };
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
	struct timespec *rem;

	rem = (struct timespec *) malloc(sizeof(struct timespec *));
	while (Running) {
		if (UpdateCheck(++t)) {
			SetOutStr();
			Print();
		}
		Sleep(T, rem);
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
	int i, t = FROMSIG(s);

	for (i = 0; i < BLKN; ++i) {
		if (blks[i].sig == t)
			UpdateBlk(i);
	}
	SetOutStr();
	Print();
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
Sleep(const struct timespec *rqtp, struct timespec *rem)
{
	if (nanosleep(rqtp, rem) && Running)
		Sleep(rem, rem);
}

void
StdoutPrint(void)
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

	for (i = U = 0; i < BLKN; ++i) {
		per = blks[i].period;
		if (per != 0 && T % per == 0 || T == 0) {
			UpdateBlk(i);
			U = 1;
		}
	}
	return U;
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
