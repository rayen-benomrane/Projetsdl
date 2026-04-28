#include "projectile.h"
#include <string.h>

void initPool(PoolProjectiles *pool) {
    memset(pool, 0, sizeof(PoolProjectiles));
}

void tirer(PoolProjectiles *pool, float x, float y, int direction, int origine) {
    int i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        if (!pool->proj[i].actif) {
            pool->proj[i].x = x;
            pool->proj[i].y = y;
            pool->proj[i].vx = (float)direction * 9.0f;
            pool->proj[i].actif = 1;
            pool->proj[i].origine = origine;
            return;
        }
    }
}

void mettreAJourPool(PoolProjectiles *pool) {
    int i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        Projectile *p = &pool->proj[i];
        if (!p->actif) continue;
        p->x += p->vx;
        if (p->x < -12 || p->x > 1100) p->actif = 0;
    }
}

void afficherPool(PoolProjectiles *pool, SDL_Renderer *re) {
    int i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        Projectile *p = &pool->proj[i];
        if (!p->actif) continue;
        SDL_Rect r = {(int)p->x, (int)p->y, 12, 6};
        if (p->origine == 0) SDL_SetRenderDrawColor(re, 0, 220, 255, 255);
        else SDL_SetRenderDrawColor(re, 255, 120, 20, 255);
        SDL_RenderFillRect(re, &r);
        SDL_SetRenderDrawColor(re, 255, 255, 255, 180);
        SDL_RenderDrawRect(re, &r);
    }
}
