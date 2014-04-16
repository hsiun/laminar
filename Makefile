CC = gcc -g -Wall
CFLAGS = -I inc/ -lpthread
SRC = src/*.c

all:

main:
	$(CC) $(CFLAGS) $(SRC) -o main

rtorret:
	$(CC) $(CFLAGS) $(SRC) -O rtorret
