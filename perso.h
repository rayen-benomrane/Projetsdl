#ifndef PERSO_H
#define PERSO_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 60
#define SOL_Y 300
#define MAX_VITESSE 0.2
#define ACCEL_MARCHE 0.005
#define FRICTION 0.9

typedef struct {
    double x, y;
    double vitesse_x, vitesse_y;
    double acceleration_x, acceleration_y;
    SDL_Texture* sprite;
    SDL_Rect posScreen;
    SDL_Rect posSprite;
    int direction;
    int state;
    int up;
    int hp;
    int score;
} Perso;

void initPerso(Perso* p, SDL_Renderer* renderer);
void handlePersoInput(Perso* p, SDL_Event event);
void updatePersoPhysics(Perso* p, Uint32 dt);
void animatePerso(Perso* p);
void renderPerso(Perso* p, SDL_Renderer* renderer);
void showHUD(Perso* p, SDL_Renderer* renderer);

#endif
