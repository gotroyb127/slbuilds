# sblocks

# installation path prefix
PREFIX = /usr/local

# includes and libs
INCS = 
LIBS = -lX11

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS   = -std=c99 -pedantic -Wall -Wno-deprecated-declarations -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# compiler and linker
CC = cc
