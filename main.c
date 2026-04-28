#include "ennemi.h"
#include "projectile.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NB_ENNEMIS 3

static int positionsX[NB_ENNEMIS]  = {150, 490, 800};
static int trajTypes[NB_ENNEMIS]   = {TRAJ_LINEAIRE, TRAJ_SINUSOIDE, TRAJ_LINEAIRE};
static int patrolMins[NB_ENNEMIS]  = {50,  300, 650};
static int patrolMaxs[NB_ENNEMIS]  = {280, 600, 950};

int main(void) {
    srand((unsigned)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return 1;

    SDL_Window   *win = SDL_CreateWindow(
        "The Babayangs Revenge",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1000, 600, 0);
    SDL_Renderer *re = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    PoolProjectiles pool;
    initPool(&pool);

    Ennemi ennemis[NB_ENNEMIS];
    int i;
    for (i = 0; i < NB_ENNEMIS; i++) {
        initEnnemi(&ennemis[i], re, trajTypes[i]);
        ennemis[i].pos.x    = positionsX[i];
        ennemis[i].posInit  = ennemis[i].pos;
        ennemis[i].patrolMin = patrolMins[i];
        ennemis[i].patrolMax = patrolMaxs[i];
    }

    /* joueur hors ecran - en attente d'integration */
    SDL_Rect joueur = {-9999, -9999, 1, 1};

    int run = 1;
    SDL_Event ev;

    while (run) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) run = 0;
            if (ev.type == SDL_KEYDOWN &&
                ev.key.keysym.sym == SDLK_ESCAPE) run = 0;
        }

        for (i = 0; i < NB_ENNEMIS; i++) {
            Ennemi *E = &ennemis[i];
            if (E->etat == NEUTRALISE) continue;
            deplacerEnnemi(E, joueur, &pool);
            animerEnnemi(E);
        }

        separerEnnemis(ennemis, NB_ENNEMIS);
        mettreAJourPool(&pool);

        SDL_SetRenderDrawColor(re, 20, 20, 30, 255);
        SDL_RenderClear(re);

        SDL_SetRenderDrawColor(re, 60, 60, 80, 255);
        SDL_RenderDrawLine(re, 0, 520, 1000, 520);

        for (i = 0; i < NB_ENNEMIS; i++)
            afficherEnnemi(&ennemis[i], re);

        afficherPool(&pool, re);

        SDL_RenderPresent(re);
        SDL_Delay(16);
    }

    for (i = 0; i < NB_ENNEMIS; i++)
        libererEnnemi(&ennemis[i]);
    SDL_DestroyRenderer(re);
    SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
