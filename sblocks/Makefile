# sblocks

include config.mk

SRC = sblocks.c
OBJ = ${SRC:.c=.o}
EXE = sblocks

all: ${EXE}

${OBJ}: config.h

.c.o:
	${CC} -c ${CFLAGS} $<

${EXE}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}


clean:
	rm -f *.o ${EXE}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${EXE} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${EXE}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${EXE}
