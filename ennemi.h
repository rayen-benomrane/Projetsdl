#ifndef ENNEMI_H
#define ENNEMI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "projectile.h"

#define WALK_FRAMES     6
#define ATTACK_FRAMES   9
#define HURT_FRAMES     2

#define ANIM_WALK   0
#define ANIM_ATTACK 1
#define ANIM_HURT   2

#define VIVANT     0
#define BLESSE     1
#define NEUTRALISE 2

#define PROJ_ENNEMI 0
#define PROJ_JOUEUR 1

#define HURT_DUREE     300
#define TRAJ_LINEAIRE  1
#define TRAJ_SINUSOIDE 2
#define MAX_ENNEMIS    3

typedef struct {
    SDL_Texture *walk[WALK_FRAMES];
    SDL_Texture *attack[ATTACK_FRAMES];
    SDL_Texture *hurt[HURT_FRAMES];
    SDL_Rect     pos;
    SDL_Rect     posInit;
    int          etat;
    int          anim;
    int          animAvantHurt;
    int          frame;
    int          direction;
    int          typeTraj;
    float        offsetY;
    Uint32       lastFrameTime;
    Uint32       lastShootTime;
    Uint32       hurtTime;
    int          vie;
    int          maxVie;
    float        vieAffichee;
    int          flashVisible;
    int          patrolMin;
    int          patrolMax;
    int          visionRange;
} Ennemi;

void initEnnemi    (Ennemi *E, SDL_Renderer *re, int level);
void deplacerEnnemi(Ennemi *E, SDL_Rect joueur, PoolProjectiles *pool);
void animerEnnemi  (Ennemi *E);
void afficherEnnemi(Ennemi *E, SDL_Renderer *re);
void libererEnnemi (Ennemi *E);
int  collisionBB   (SDL_Rect a, SDL_Rect b);
void separerEnnemis(Ennemi ennemis[], int n);

#endif
