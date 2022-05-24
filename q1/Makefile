SHELL = /bin/sh

OBJS = phrases.c
CFLAGS = -Wall -Werror
CC = gcc
INCLUDE =
LIBS =

phrases: phrases.c
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}

clean: 
	-rm -f *.o core *.core phrases

.o: ${CC} ${CFLAGS} ${INCLUDES} -c $<