SHELL = /bin/sh

OBJS = addmx.c
CFLAGS = -Wall -Werror -fsanitize=address -fsanitize=undefined
CC = gcc
INCLUDE =
LIBS =

addmx:
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}

clean: 
	-rm -f *.o core *.core addmx

.o: ${CC} ${CFLAGS} ${INCLUDES} -c $<