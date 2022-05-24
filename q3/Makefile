SHELL = /bin/sh

OBJS = cypher.c
CFLAGS = -Wall -Werror
CC = gcc
INCLUDE =
LIBS =

cypher: cypher.c
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}

clean: 
	-rm -f *.o core *.core cypher

.o: ${CC} ${CFLAGS} ${INCLUDES} -c $<