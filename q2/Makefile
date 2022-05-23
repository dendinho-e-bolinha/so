SHELL = /bin/sh

OBJS = addmx.c
CFLAGS = -Wall -Werror
CC = gcc
INCLUDE =
LIBS =

addmx: addmx.c
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}

clean: 
	-rm -f *.o core *.core addmx

.o: ${CC} ${CFLAGS} ${INCLUDES} -c $<