SHELL = /bin/sh

OBJS = phrases.c
CFLAGS = -Wall -Werror
CC = gcc
INCLUDE =
LIBS =

phrase:
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}

clean: 
	-rm -f *.o core *.core phrase

.o: ${CC} ${CFLAGS} ${INCLUDES} -c $<