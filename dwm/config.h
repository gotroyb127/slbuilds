/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 3;        /* border pixel of windows */
static const unsigned int snap      = 15;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 0;        /* 0 means bottom bar */
static const char *fonts[]          = { "Fira Code Medium:size=10" };
static const char col_gray0[]       = "#000000";
static const char col_gray1[]       = "#111111";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#999999";
static const char col_gray4[]       = "#eeeeee";
static const char col_gray5[]       = "#ffffff";
static const char col_SelCl[]       = "eeeeeee"; /* Selected Client */
static const char col_SelTag[]      = "#333333"; /* Selected Tag */
static const char col_cyan[]        = "#005577";
static const char *colors[][3]      = {
	/*                   fg         bg         border   */
	[SchemeNorm]     = { NULL,      NULL,      col_gray2 }, /* only border affects */
	[SchemeSel]      = { NULL,      NULL,      col_SelCl }, /* only border affects */
	[SchemeStatus]   = { "#cccccc", col_gray0, NULL      }, /* Statusbar right (border unused) */
	[SchemeTagsSel]  = { col_gray4, col_gray0, NULL      }, /* Tagbar left selected (border unused) */
	[SchemeTagsNorm] = { col_gray3, col_gray0, NULL      }, /* Tagbar left unselected (border unused) */
	[SchemeInfoSel]  = { col_gray5, col_gray0, NULL      }, /* infobar middle selected (border unused) */
	[SchemeInfoNorm] = { col_gray3, col_gray0, NULL      }, /* infobar middle  unselected (border unused) */
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask  isfloating  add2borderw  monitor */
	{ "teeworlds", NULL,      NULL,       0,         0,          -borderpx,   -1 },
	{ "DDNet",    NULL,       NULL,       0,         0,          -borderpx,   -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int dirs[3]     = { DirRotHor, DirVer, DirVer }; /* tiling dirs */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */
                                        /* for smallmonocle: */
static const float scalefactorx = 0.70, /* width  = (mon_w) * (mfact ^ scalefactorx) */
                   scalefactory = 0.0;  /* height = (mon_h) * (mfact ^ scalefactory) */

enum { Tiled, CenteredMaster, Monocle, SmallMonocle, Floating }; /* layouts by name */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "<||>",     tile },    /* first entry is default */
	{ "[||]",     centeredmaster },
	{ "[M]",      monocle },
	{ "<M>",      smallmonocle },
	{ "><>",      NULL },    /* no layout function means floating behavior */
};

/* key definitions */
#define MODKEY  Mod4Mask
#define MODKEY2 Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },
#define TILEKEYS(MOD,KEY,G,M,S) \
	{ MOD, KEY, setdirs, {.v = (int[]) { INC(G * +1), INC(M * +1), INC(S * +1) } } },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd)        { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }
#define SHCMD_SIG(N,cmd)  SHCMD(cmd " && kill -$((31-"N")) $(pidof sblocks)")
#define MIXER(args)       "amixer -Mq set " args
#define PLAYER(args)      "Player.sh " args

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, NULL };
static const char *termcmd[]  = { "st", "tmux", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */

	{ MODKEY,                       XK_j,            focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,            focusstack,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_j,            pushdown,       {0} },
	{ MODKEY|ShiftMask,             XK_k,            pushup,         {0} },
	{ MODKEY,                       XK_i,            incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,            incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,            setmfact,       {.f = -0.1} },
	{ MODKEY,                       XK_l,            setmfact,       {.f = +0.1} },
	{ MODKEY|ControlMask,           XK_h,            setmfact,       {.f = -0.025} },
	{ MODKEY|ControlMask,           XK_l,            setmfact,       {.f = +0.025} },
	{ MODKEY,                       XK_o,            setmfact,       {.f = 1.5} },
	{ MODKEY|ControlMask,           XK_o,            setmfact,       {.f = 1.33} },
	{ MODKEY|ShiftMask,             XK_h,            setcfact,       {.f = -0.5} },
	{ MODKEY|ShiftMask,             XK_l,            setcfact,       {.f = +0.5} },
	{ MODKEY|ShiftMask|ControlMask, XK_h,            setcfact,       {.f = -0.25} },
	{ MODKEY|ShiftMask|ControlMask, XK_l,            setcfact,       {.f = +0.25} },
	{ MODKEY|ShiftMask,             XK_o,            setcfact,       {.f =  0.0} },
	{ MODKEY,                       XK_z,            zoom,           {0} },
	{ MODKEY|ShiftMask,             XK_Return,       zoom,           {0} },
	{ MODKEY,                       XK_Tab,          view,           {0} },
	{ MODKEY,                       XK_q,            killclient,     {0} },
	{ MODKEY,                       XK_space,        setlayout,      {0} },
	{ MODKEY,                       XK_x,            setlayout,      {.v = &layouts[Tiled]} },
	{ MODKEY|ShiftMask|ControlMask, XK_f,            setlayout,      {.v = &layouts[Floating]} },
	{ MODKEY,                       XK_m,            setlayout,      {.v = &layouts[Monocle]} },
	{ MODKEY,                       XK_n,            setlayout,      {.v = &layouts[SmallMonocle]} },
	{ MODKEY,                       XK_c,            setlayout,      {.v = &layouts[CenteredMaster]} },
	{ MODKEY,                       XK_f,            setlayoutnobar, {.v = &layouts[Monocle]} },
	{ MODKEY|ShiftMask,             XK_f,            setlayoutnobar, {.v = &layouts[SmallMonocle]} },
	TILEKEYS(MODKEY,                       XK_r,                     1, 0, 0)
	TILEKEYS(MODKEY|ControlMask,           XK_r,                     0, 1, 0)
	TILEKEYS(MODKEY|ShiftMask,             XK_r,                     0, 0, 1)
	TILEKEYS(MODKEY|ShiftMask|ControlMask, XK_r,                     1, 1, 1)
	{ MODKEY|ControlMask,           XK_f,            setdirs,        {.v = (int[]) { DirHor,    DirVer, DirVer } } },
	{ MODKEY|ControlMask,           XK_a,            setdirs,        {.v = (int[]) { DirRotHor, DirVer, DirVer } } },
	{ MODKEY|ControlMask,           XK_s,            setdirs,        {.v = (int[]) { DirVer,    DirHor, DirHor } } },
	{ MODKEY|ControlMask,           XK_d,            setdirs,        {.v = (int[]) { DirRotVer, DirHor, DirHor } } },
	{ MODKEY,                       XK_b,            togglebar,      {0} },
	{ MODKEY|ControlMask,           XK_i,            toggleisperm,   {0} },
	{ MODKEY,                       XK_g,            togglefullscr,  {0} },
	{ MODKEY|ShiftMask,             XK_space,        togglefloating, {0} },
	{ MODKEY,                       XK_comma,        focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period,       focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,        tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,       tagmon,         {.i = +1 } },
	{ MODKEY,                       XK_0,            view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,            tag,            {.ui = ~0 } },
	TAGKEYS(                        XK_1,                            0)
	TAGKEYS(                        XK_2,                            1)
	TAGKEYS(                        XK_3,                            2)
	TAGKEYS(                        XK_4,                            3)
	TAGKEYS(                        XK_5,                            4)
	TAGKEYS(                        XK_6,                            5)
	TAGKEYS(                        XK_7,                            6)
	TAGKEYS(                        XK_8,                            7)
	TAGKEYS(                        XK_9,                            8)
	{ MODKEY|ShiftMask,             XK_e,            quit,           {0} },
	{ MODKEY|ShiftMask|ControlMask, XK_e,            quit,           {1} },

	{ MODKEY,                       XK_p,            spawn,          {.v = dmenucmd } },
	{ MODKEY|MODKEY2,               XK_x,            spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_Return,       spawn,          {.v = termcmd } },
	{ MODKEY|ControlMask,           XK_Return,       spawn,          SHCMD("tmux has 2> /dev/null && st tmux attach") },
	{ MODKEY|MODKEY2|ControlMask,   XK_x,            spawn,          SHCMD("tmux has 2> /dev/null && st tmux attach") },
	{ MODKEY|MODKEY2,               XK_c,            spawn,          SHCMD("st tmux new $SHELL -ic lf") },
	{ MODKEY|MODKEY2,               XK_F4,           spawn,          SHCMD("PowerOptions") },
	{ ShiftMask,                    XK_Caps_Lock,    spawn,          SHCMD_SIG("2", ":") },
	{ MODKEY,                       XK_Home,         spawn,          SHCMD_SIG("5", ":") },
	{ MODKEY|ControlMask,           XK_F5,           spawn,          SHCMD_SIG("4", ":") },
	{ MODKEY,                       XK_F5,           spawn,          SHCMD_SIG("4", MIXER("Capture toggle")) },
	{ MODKEY|ShiftMask,             XK_F6,           spawn,          SHCMD_SIG("4", MIXER("Master 15%-   ")) },
	{ MODKEY,                       XK_F6,           spawn,          SHCMD_SIG("4", MIXER("Master 5%-    ")) },
	{ MODKEY|ControlMask,           XK_F6,           spawn,          SHCMD_SIG("4", MIXER("Master 1%-    ")) },
	{ MODKEY,                       XK_F7,           spawn,          SHCMD_SIG("4", MIXER("Master toggle ")) },
	{ MODKEY|ShiftMask,             XK_F7,           spawn,          SHCMD_SIG("4", MIXER("Master 70%    ")) },
	{ MODKEY|ControlMask,           XK_F7,           spawn,          SHCMD_SIG("4", MIXER("Master 35%    ")) },
	{ MODKEY|ControlMask,           XK_F8,           spawn,          SHCMD_SIG("4", MIXER("Master 1%+    ")) },
	{ MODKEY,                       XK_F8,           spawn,          SHCMD_SIG("4", MIXER("Master 5%+    ")) },
	{ MODKEY|ShiftMask,             XK_F8,           spawn,          SHCMD_SIG("4", MIXER("Master 15%+   ")) },

#include <X11/XF86keysym.h>
	{ ShiftMask,        XF86XK_AudioLowerVolume,     spawn,          SHCMD_SIG("4", MIXER("Master 15%-   ")) },
	{ 0,                XF86XK_AudioLowerVolume,     spawn,          SHCMD_SIG("4", MIXER("Master 5%-    ")) },
	{ ControlMask,      XF86XK_AudioLowerVolume,     spawn,          SHCMD_SIG("4", MIXER("Master 1%-    ")) },
	{ 0,                XF86XK_AudioMute,            spawn,          SHCMD_SIG("4", MIXER("Master toggle ")) },
	{ ShiftMask,        XF86XK_AudioMute,            spawn,          SHCMD_SIG("4", MIXER("Master 70%    ")) },
	{ ControlMask,      XF86XK_AudioMute,            spawn,          SHCMD_SIG("4", MIXER("Master 35%    ")) },
	{ ControlMask,      XF86XK_AudioRaiseVolume,     spawn,          SHCMD_SIG("4", MIXER("Master 1%+    ")) },
	{ 0,                XF86XK_AudioRaiseVolume,     spawn,          SHCMD_SIG("4", MIXER("Master 5%+    ")) },
	{ ShiftMask,        XF86XK_AudioRaiseVolume,     spawn,          SHCMD_SIG("4", MIXER("Master 15%+   ")) },

	{ MODKEY|ControlMask,           XK_F10,          spawn,          SHCMD_SIG("1", PLAYER("pause-after 1")) },
	{ MODKEY|ShiftMask|ControlMask, XK_F9,           spawn,          SHCMD_SIG("1", PLAYER("previous     ")) },
	{ MODKEY|ShiftMask,             XK_F9,           spawn,          SHCMD_SIG("1", PLAYER("position- 60 ")) },
	{ MODKEY,                       XK_F9,           spawn,          SHCMD_SIG("1", PLAYER("position- 5  ")) },
	{ MODKEY|ControlMask,           XK_F9,           spawn,          SHCMD_SIG("1", PLAYER("position- 1  ")) },
	{ MODKEY,                       XK_F10,          spawn,          SHCMD_SIG("1", PLAYER("play-pause   ")) },
	{ MODKEY|ControlMask,           XK_F11,          spawn,          SHCMD_SIG("1", PLAYER("position+ 1  ")) },
	{ MODKEY,                       XK_F11,          spawn,          SHCMD_SIG("1", PLAYER("position+ 5  ")) },
	{ MODKEY|ShiftMask,             XK_F11,          spawn,          SHCMD_SIG("1", PLAYER("position+ 60 ")) },
	{ MODKEY|ShiftMask|ControlMask, XK_F11,          spawn,          SHCMD_SIG("1", PLAYER("next         ")) },
	{ MODKEY|ControlMask,           XK_bracketleft,  spawn,          SHCMD_SIG("1", PLAYER("speed- 0.01  ")) },
	{ MODKEY|ShiftMask,             XK_bracketleft,  spawn,          SHCMD_SIG("1", PLAYER("speed- 0.05  ")) },
	{ MODKEY,                       XK_bracketleft,  spawn,          SHCMD_SIG("1", PLAYER("speed- 0.1   ")) },
	{ MODKEY,                       XK_equal,        spawn,          SHCMD_SIG("1", PLAYER("speed  1     ")) },
	{ MODKEY,                       XK_bracketright, spawn,          SHCMD_SIG("1", PLAYER("speed+ 0.1   ")) },
	{ MODKEY|ShiftMask,             XK_bracketright, spawn,          SHCMD_SIG("1", PLAYER("speed+ 0.05  ")) },
	{ MODKEY|ControlMask,           XK_bracketright, spawn,          SHCMD_SIG("1", PLAYER("speed+ 0.01  ")) },
	{ MODKEY,                       XK_minus,        spawn,          SHCMD_SIG("1", PLAYER("loop- 1      ")) },
	{ MODKEY|ControlMask,           XK_minus,        spawn,          SHCMD_SIG("1", PLAYER("loop+ 1      ")) },
	{ MODKEY|ShiftMask,             XK_minus,        spawn,          SHCMD_SIG("1", PLAYER("positionm    ")) },
	{ MODKEY|ShiftMask,             XK_equal,        spawn,          SHCMD_SIG("1", PLAYER("quit-wl      ")) },
	{ MODKEY,                       XK_v,            spawn,          SHCMD("clipsavedshow -l 20") },
	{ MODKEY|MODKEY2,               XK_v,            spawn,          SHCMD("clipsavedshow --rm -l 20") },
	{ MODKEY|MODKEY2,               XK_f,            spawn,          SHCMD("gepasl ~/Documents/havsepas dmenu -i -l 20"), },
	{ MODKEY|MODKEY2,               XK_m,            spawn,          SHCMD("st Music.sh") },
	{ MODKEY|MODKEY2,               XK_h,            spawn,          SHCMD("st htop") },
	{ MODKEY|MODKEY2,               XK_g,            spawn,          SHCMD("gsimplecal") },
	{ MODKEY|MODKEY2,               XK_s,            spawn,          SHCMD("firefox") },
	{ MODKEY|MODKEY2,               XK_a,            spawn,          SHCMD("firefox -P") },
	{ MODKEY|MODKEY2,               XK_d,            spawn,          SHCMD("firefox -P 1Private") },
	{ MODKEY|MODKEY2|ControlMask,   XK_a,            spawn,          SHCMD("firefox -P --private-window") },
	{ MODKEY,                       XK_Print,        spawn,          SHCMD("PrintScreen") },
	{ MODKEY|ShiftMask,             XK_Print,        spawn,          SHCMD("PrintScreen window") },
	{ MODKEY|MODKEY2,               XK_p,            spawn,          SHCMD("xfce4-appfinder") },
	{ MODKEY|ControlMask,           XK_c,            spawn,          SHCMD("xcalib -o 1 -i -a") },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask        button          function        argument */
	{ ClkLtSymbol,          MODKEY,           Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,                Button1,        setlayout,      {.v = &layouts[Tiled]} },
	{ ClkLtSymbol,          0,                Button2,        setlayout,      {.v = &layouts[Floating]} },
	{ ClkLtSymbol,          0,                Button3,        setlayout,      {.v = &layouts[SmallMonocle]} },
	{ ClkLtSymbol,          0,                Button4,        setlayout,      {.v = &layouts[Monocle]} },
	{ ClkLtSymbol,          0,                Button5,        setlayout,      {.v = &layouts[CenteredMaster]} },
	{ ClkLtSymbol,          0,                      9,        setlayout,      {.v = &layouts[Monocle]} },
	{ ClkLtSymbol,          0,                      8,        setlayout,      {.v = &layouts[CenteredMaster]} },
	{ ClkRootWin,           0,                Button4,        togglebar,      {0} },
	{ ClkRootWin,           0,                Button5,        togglebar,      {0} },
	{ ClkWinTitle,          0,                Button2,        setmfact,       {.f = 1.5} },
	{ ClkWinTitle,          0,                Button4,        setmfact,       {.f = +0.05} },
	{ ClkWinTitle,          0,                Button5,        setmfact,       {.f = -0.05} },
	{ ClkWinTitle,          MODKEY,           Button3,        setcfact,       {.f =  0.00} },
	{ ClkWinTitle,          MODKEY,           Button4,        setcfact,       {.f = +0.25} },
	{ ClkWinTitle,          MODKEY,           Button5,        setcfact,       {.f = -0.25} },
	{ ClkStatusText,        0,                Button2,        spawn,          SHCMD("xfce4-appfinder") },
	{ ClkClientWin,         MODKEY,           Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,           Button2,        zoom,           {0} },
	{ ClkClientWin,         MODKEY|ShiftMask, Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,           Button3,        resizemouse,    {0} },
	{ ClkClientWin,         MODKEY,           Button4,        pushup,         {0} },
	{ ClkClientWin,         MODKEY,           Button5,        pushdown,       {0} },
	{ ClkTagBar,            0,                Button1,        view,           {0} },
	{ ClkTagBar,            0,                Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,           Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,           Button3,        toggletag,      {0} },
	{ ClkTagBar,            0,                Button4,        incnmaster,     {.i = +1} },
	{ ClkTagBar,            0,                Button5,        incnmaster,     {.i = -1} },
};
