/* user and group to drop privileges to */
static const char *user  = "nobody";
static const char *group = "nobody";

static const char *colorname[NUMCOLS] = {
	[INIT] =   "#663300",   /* after initialization */
	[INPUT] =  "#222738",   /* during input */
	[FAILED] = "#6C1616",   /* wrong password */
};

/* treat a cleared input like a wrong password (color) */
static const int failonclear = 0;
