SHELL = /bin/sh

OBJS = cypher.c
CFLAGS = -fsanitize=address -fsanitize=undefined -g
CC = gcc
INCLUDE =
LIBS =

cypher: cypher.c
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}

clean: 
	-rm -f *.o core *.core cypher

.o: ${CC} ${CFLAGS} ${INCLUDES} -c $<