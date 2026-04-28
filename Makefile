jeu: main.o ennemi.o projectile.o
	gcc main.o ennemi.o projectile.o -o jeu -lSDL2 -lSDL2_image -lm

main.o: main.c
	gcc -Wall -Wextra -g -c main.c

ennemi.o: ennemi.c
	gcc -Wall -Wextra -g -c ennemi.c

projectile.o: projectile.c
	gcc -Wall -Wextra -g -c projectile.c

clean:
	rm -f *.o jeu
