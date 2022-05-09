SHELL = /bin/sh

OBJS = cypher.c
CFLAGS = -Wall -Werror
CC = gcc
INCLUDE =
LIBS =

phrase:
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}

clean: 
	-rm -f *.o core *.core phrase

.o: ${CC} ${CFLAGS} ${INCLUDES} -c $<