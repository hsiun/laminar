CC = gcc -g -Wall
CFLAGS = -I inc/ 
SRC = src/*.c

laminar:main.o rtorret.o
	$(CC) $(CFLAGS) $(SRC) -o laminar

main.o:
	$(CC) $(CFLAGS) $(SRC) -o ./src/main.o

rtorret.o:
	$(CC) $(CFLAGS) $(SRC) -o ./src/rtorret.o
