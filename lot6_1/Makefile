puzzle : main.o puzzle.o
	gcc puzzle.o main.o -o puzzle -lSDL2_ttf -lSDL2_mixer -lSDL2_image -lSDL2 -lm -g

main.o : main.c
	gcc -c main.c `sdl2-config --cflags`

puzzle.o : puzzle.c
	gcc -c puzzle.c `sdl2-config --cflags`

clean:
	rm -f *.o puzzle

run: puzzle
	./puzzle
