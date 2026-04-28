#include "ennemi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void initEnnemi(Ennemi *E, SDL_Renderer *re, int level) {
    const char *fWalk[WALK_FRAMES] = {
        "assets/walk1.png", "assets/walk2.png", "assets/walk3.png",
        "assets/walk4.png", "assets/walk5.png", "assets/walk7.png"
    };
    int i;
    char path[64];
    for (i = 0; i < WALK_FRAMES; i++)
        E->walk[i] = IMG_LoadTexture(re, fWalk[i]);
    for (i = 0; i < ATTACK_FRAMES; i++) {
        sprintf(path, "assets/attack%d.png", i + 1);
        E->attack[i] = IMG_LoadTexture(re, path);
    }
    E->hurt[0] = IMG_LoadTexture(re, "assets/hurt1.png");
    E->hurt[1] = IMG_LoadTexture(re, "assets/hurt2.png");
    E->pos           = (SDL_Rect){700, 420, 100, 100};
    E->posInit       = E->pos;
    E->etat          = VIVANT;
    E->anim          = ANIM_WALK;
    E->animAvantHurt = ANIM_WALK;
    E->frame         = 0;
    E->direction     = -1;
    E->typeTraj      = level;
    E->offsetY       = 0;
    E->lastFrameTime = SDL_GetTicks();
    E->lastShootTime = 0;
    E->hurtTime      = 0;
    E->vie           = 100;
    E->maxVie        = 100;
    E->vieAffichee   = 100.0f;
    E->flashVisible  = 1;
    E->patrolMin     = 50;
    E->patrolMax     = 900;
    E->visionRange   = 350;
}

int collisionBB(SDL_Rect a, SDL_Rect b) {
    return SDL_HasIntersection(&a, &b);
}

void deplacerEnnemi(Ennemi *E, SDL_Rect joueur, PoolProjectiles *pool) {
    int eCx, jCx, dist, speed, i;
    Uint32 now;
    float bx, by;
    if (E->etat == NEUTRALISE) return;
    if (E->anim == ANIM_HURT) return;

    eCx  = E->pos.x + E->pos.w / 2;
    jCx  = joueur.x + joueur.w / 2;
    dist = abs(eCx - jCx);
    now  = SDL_GetTicks();

    /* joueur dans le champ de vision : AGGRESSIF */
    if (dist <= E->visionRange) {
        E->direction = (jCx < eCx) ? -1 : 1;
        if (dist < 120) {
            if (E->anim != ANIM_ATTACK) { E->anim = ANIM_ATTACK; E->frame = 0; }
        } else {
            E->anim = ANIM_ATTACK;
            if (now - E->lastShootTime >= 1200) {
                bx = (E->direction == 1)
                     ? (float)(E->pos.x + E->pos.w)
                     : (float)(E->pos.x - 12);
                by = (float)(E->pos.y + E->pos.h / 2 - 3);
                for (i = 0; i < MAX_PROJECTILES; i++) {
                    if (!pool->proj[i].actif) {
                        pool->proj[i].x       = bx;
                        pool->proj[i].y       = by;
                        pool->proj[i].vx      = E->direction * 9.0f;
                        pool->proj[i].actif   = 1;
                        pool->proj[i].origine = PROJ_ENNEMI;
                        break;
                    }
                }
                E->lastShootTime = now;
            }
            speed = 1 + E->typeTraj;
            E->pos.x += speed * E->direction;
        }

    /* joueur hors vision : aller-retour dans la zone */
    } else {
        speed = 1 + E->typeTraj;
        E->anim = ANIM_WALK;

        E->pos.x += speed * E->direction;

        /* rebondir aux bornes de la zone */
        if (E->pos.x <= E->patrolMin) {
            E->pos.x  = E->patrolMin;
            E->direction = 1;
        }
        if (E->pos.x >= E->patrolMax) {
            E->pos.x  = E->patrolMax;
            E->direction = -1;
        }

        if (E->typeTraj == TRAJ_SINUSOIDE) {
            E->offsetY = (float)(20.0 * sin(now * 0.003));
            E->pos.y   = 420 + (int)E->offsetY;
        }
    }
}

void animerEnnemi(Ennemi *E) {
    Uint32 now, delay;
    int max;
    if (E->etat == NEUTRALISE) return;
    now = SDL_GetTicks();
    if (E->anim == ANIM_HURT) {
        E->flashVisible = ((now - E->hurtTime) / 50) % 2;
        if (now - E->hurtTime >= HURT_DUREE) {
            E->anim  = E->animAvantHurt;
            E->frame = 0;
            E->flashVisible = 1;
        } else {
            E->frame = ((now - E->hurtTime) * HURT_FRAMES) / HURT_DUREE;
            if (E->frame >= HURT_FRAMES) E->frame = HURT_FRAMES - 1;
        }
        return;
    }
    delay = (E->anim == ANIM_ATTACK) ? 80 : 120;
    if (now > E->lastFrameTime + delay) {
        E->frame++;
        max = (E->anim == ANIM_WALK) ? WALK_FRAMES : ATTACK_FRAMES;
        if (E->frame >= max) E->frame = 0;
        E->lastFrameTime = now;
    }
    if (E->vieAffichee > (float)E->vie)
        E->vieAffichee -= 1.5f;
    if (E->vieAffichee < (float)E->vie)
        E->vieAffichee = (float)E->vie;
}

void afficherEnnemi(Ennemi *E, SDL_Renderer *re) {
    SDL_Texture *t;
    SDL_RendererFlip flip;
    int barW, fillW, cx, cy, rx, ry, x, y;
    SDL_Rect barBg, barFill;

    if (E->etat == NEUTRALISE) return;

    /* ombre elliptique au sol */
    cx = E->pos.x + E->pos.w / 2;
    cy = E->pos.y + E->pos.h - 4;
    rx = 30; ry = 8;
    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(re, 0, 0, 0, 60);
    for (y = -ry; y <= ry; y++)
        for (x = -rx; x <= rx; x++) {
            float fx = (float)x / rx;
            float fy = (float)y / ry;
            if (fx*fx + fy*fy <= 1.0f)
                SDL_RenderDrawPoint(re, cx + x, cy + y);
        }
    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_NONE);

    /* flash pendant HURT */
    if (!E->flashVisible) return;

    if (E->anim == ANIM_HURT) {
        if (E->frame < 0 || E->frame >= HURT_FRAMES) E->frame = 0;
        t = E->hurt[E->frame];
    } else if (E->anim == ANIM_WALK) {
        if (E->frame < 0 || E->frame >= WALK_FRAMES) E->frame = 0;
        t = E->walk[E->frame];
    } else {
        if (E->frame < 0 || E->frame >= ATTACK_FRAMES) E->frame = 0;
        t = E->attack[E->frame];
    }
    if (!t) return;
    flip = (E->direction == 1) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_RenderCopyEx(re, t, NULL, &E->pos, 0, NULL, flip);

    /* barre de vie avec vieAffichee (interpolation douce) */
    barW  = 80;
    fillW = ((int)E->vieAffichee * barW) / E->maxVie;
    if (fillW < 0) fillW = 0;
    barBg   = (SDL_Rect){E->pos.x + (E->pos.w - barW) / 2, E->pos.y - 14, barW, 8};
    barFill = (SDL_Rect){barBg.x, barBg.y, fillW, 8};
    SDL_SetRenderDrawColor(re, 80, 0, 0, 255);
    SDL_RenderFillRect(re, &barBg);
    if (E->vie > 60)
        SDL_SetRenderDrawColor(re, 50, 200, 50, 255);
    else if (E->vie > 30)
        SDL_SetRenderDrawColor(re, 220, 140, 30, 255);
    else
        SDL_SetRenderDrawColor(re, 220, 40, 40, 255);
    SDL_RenderFillRect(re, &barFill);
    SDL_SetRenderDrawColor(re, 255, 255, 255, 200);
    SDL_RenderDrawRect(re, &barBg);
}

void libererEnnemi(Ennemi *E) {
    int i;
    for (i = 0; i < WALK_FRAMES;   i++) if (E->walk[i])   SDL_DestroyTexture(E->walk[i]);
    for (i = 0; i < ATTACK_FRAMES; i++) if (E->attack[i]) SDL_DestroyTexture(E->attack[i]);
    for (i = 0; i < HURT_FRAMES;   i++) if (E->hurt[i])   SDL_DestroyTexture(E->hurt[i]);
}

/* positions de depart pour chaque ennemi */
static int positionsX[3] = {700, 500, 850};
static int trajTypes[3]  = {TRAJ_LINEAIRE, TRAJ_SINUSOIDE, TRAJ_LINEAIRE};

void initTousEnnemis(Ennemi ennemis[], int n, SDL_Renderer *re) {
    int i;
    for (i = 0; i < n; i++) {
        initEnnemi(&ennemis[i], re, trajTypes[i]);
        ennemis[i].pos.x    = positionsX[i];
        ennemis[i].posInit  = ennemis[i].pos;
    }
}

void deplacerTousEnnemis(Ennemi ennemis[], int n, SDL_Rect joueur, PoolProjectiles *pool) {
    int i;
    for (i = 0; i < n; i++)
        if (ennemis[i].etat != NEUTRALISE)
            deplacerEnnemi(&ennemis[i], joueur, pool);
}

void animerTousEnnemis(Ennemi ennemis[], int n) {
    int i;
    for (i = 0; i < n; i++)
        animerEnnemi(&ennemis[i]);
}

void afficherTousEnnemis(Ennemi ennemis[], int n, SDL_Renderer *re) {
    int i;
    for (i = 0; i < n; i++)
        afficherEnnemi(&ennemis[i], re);
}

void libererTousEnnemis(Ennemi ennemis[], int n) {
    int i;
    for (i = 0; i < n; i++)
        libererEnnemi(&ennemis[i]);
}

void separerEnnemis(Ennemi ennemis[], int n) {
    int i, j;
    int minDist = 90;
    for (i = 0; i < n; i++) {
        if (ennemis[i].etat == NEUTRALISE) continue;
        for (j = i + 1; j < n; j++) {
            if (ennemis[j].etat == NEUTRALISE) continue;
            int dx = ennemis[j].pos.x - ennemis[i].pos.x;
            if (dx == 0) dx = 1;
            int adx = abs(dx);
            if (adx < minDist) {
                int push = (minDist - adx) / 2 + 1;
                if (dx > 0) {
                    ennemis[i].pos.x -= push;
                    ennemis[j].pos.x += push;
                } else {
                    ennemis[i].pos.x += push;
                    ennemis[j].pos.x -= push;
                }
            }
        }
    }
}
