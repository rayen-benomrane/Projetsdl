prog: main.o lot.o
	gcc main.o lot.o -o prog -lSDL2 -lSDL2_image
main.o: main.c lot.h
	gcc -c main.c
lot.o: lot.c lot.h
	gcc -c lot.c
