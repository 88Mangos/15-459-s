CFLAGS=-fwrapv -Wall -Wextra -Werror -g
CC=gcc -std=c99 -pedantic
LIB=lib/*.h

default: pcs1 pcs2 qvm

pcs1: pcs1.c
	$(CC) $(CFLAGS) $(LIB) -o pcs1 pcs1.c

pcs2: pcs2.c
	$(CC) $(CFLAGS) $(LIB) -o pcs2 pcs2.c

qvm: qvm.c
	$(CC) $(CFLAGS) $(LIB) -o qvm qvm.c

clean:
	rm -Rf pcs1 pcs2 qvm *.dSYM
