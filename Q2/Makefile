SHELL = /bin/sh

OBJS = addmx.c
CFLAGS = -Wall -Werror -g
CC = gcc
INCLUDE =
LIBS =

addmx: addmx.c
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}

clean: 
	-rm -f *.o core *.core addmx

.o: ${CC} ${CFLAGS} ${INCLUDES} -c $<