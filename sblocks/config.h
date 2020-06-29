/* The following Cmd strings are passed to sh to be evaluated. Signal X is used to update 
 * the appropriate block when dwmblock receives TOSIG(X) (see macro). Defaults to (31 - (X)),
 * no need for RTMIN signals that Linux provide, as it overrides other signals (unsure if it is safe).
 */

static const Blk blks[] = {
    /*    before    command                                       after  period    signal */
        { "",       "~/.local/scripts/statusblocks/player.sh",    "",    1,             1},
        { "    ",   "~/.local/scripts/statusblocks/volume.sh",    "",    10,            4},
        { "",       "~/.local/scripts/statusblocks/kblayout.sh",  "",    10,            2},
        { "  ",     "~/.local/scripts/statusblocks/battery.sh",   "",    10,            2},
        { " ",      "~/.local/scripts/statusblocks/network.sh",   "",    10,            5},
        { "    ",   "~/.local/scripts/statusblocks/date.sh",      "",    1,             2},
};
