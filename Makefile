CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = -lSDL2 -lSDL2_image -lm
OBJ = main.o perso.o
EXEC = jeu

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LIBS)

main.o: main.c perso.h
	$(CC) $(CFLAGS) -c main.c

perso.o: perso.c perso.h
	$(CC) $(CFLAGS) -c perso.c

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
