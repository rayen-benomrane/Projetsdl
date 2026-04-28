#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SDL2/SDL.h>

#define MAX_PROJECTILES 32

typedef struct {
    float x, y;
    float vx;
    int actif;
    int origine;
} Projectile;

typedef struct {
    Projectile proj[MAX_PROJECTILES];
} PoolProjectiles;

void initPool(PoolProjectiles *pool);
void tirer(PoolProjectiles *pool, float x, float y, int direction, int origine);
void mettreAJourPool(PoolProjectiles *pool);
void afficherPool(PoolProjectiles *pool, SDL_Renderer *re);

#endif
