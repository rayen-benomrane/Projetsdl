#include "perso.h"

void initPerso(Perso* p, SDL_Renderer* renderer) {
    p->x = 50.0;
    p->y = SOL_Y;
    p->vitesse_x = 0;
    p->vitesse_y = 0;
    p->acceleration_x = 0;
    p->acceleration_y = 0.005;
    p->up = 0;
    p->hp = 100;
    p->score = 0;
    p->direction = 0;
    p->state = 0;
    SDL_Surface* surface = IMG_Load("perso.png");
    if (surface) {
        SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 0, 0));
        p->posSprite.w = surface->w / 8;
        p->posSprite.h = surface->h / 4;
        p->sprite = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    p->posSprite.x = 0;
    p->posSprite.y = 0;
    p->posScreen.w = p->posSprite.w;
    p->posScreen.h = p->posSprite.h;
}

void handlePersoInput(Perso* p, SDL_Event event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_RIGHT:
                p->acceleration_x = ACCEL_MARCHE;
                p->direction = 0;
                if(p->up == 0) p->state = 1;
                break;
            case SDLK_LEFT:
                p->acceleration_x = -ACCEL_MARCHE;
                p->direction = 1;
                if(p->up == 0) p->state = 1;
                break;
            case SDLK_UP:
                if (p->up == 0) {
                    p->vitesse_y = -1.0;
                    p->up = 1;
                    p->state = 2;
                }
                break;
            case SDLK_a:
                p->state = 3;
                p->score = p->score + 5;
                break;
        }
    } else if (event.type == SDL_KEYUP) {
        if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_LEFT) {
            p->acceleration_x = 0;
            p->vitesse_x = 0;
            if(p->up == 0) p->state = 0;
        } else if (event.key.keysym.sym == SDLK_a) {
            if(p->up == 0) p->state = 0;
        }
    }
}

void updatePersoPhysics(Perso* p, Uint32 dt) {
    double dx = 0.5 * p->acceleration_x * dt * dt + p->vitesse_x * dt;
    double dy = 0.5 * p->acceleration_y * dt * dt + p->vitesse_y * dt;
    p->x += dx;
    p->y += dy;
    p->vitesse_x += p->acceleration_x * dt;
    p->vitesse_y += p->acceleration_y * dt;
    if (p->acceleration_x == 0) p->vitesse_x *= FRICTION;
    if (p->vitesse_x > MAX_VITESSE) p->vitesse_x = MAX_VITESSE;
    if (p->vitesse_x < -MAX_VITESSE) p->vitesse_x = -MAX_VITESSE;
    if (p->y >= SOL_Y) {
        p->y = SOL_Y;
        p->vitesse_y = 0;
        p->up = 0;
        if (p->state == 2) p->state = 0;
    }
    p->posScreen.x = (int)p->x;
    p->posScreen.y = (int)p->y;
}

void animatePerso(Perso* p) {
    p->posSprite.y = p->state * p->posSprite.h;
    if (p->posSprite.x >= (7 * p->posSprite.w)) {
        p->posSprite.x = 0;
    } else {
        p->posSprite.x += p->posSprite.w;
    }
}

void renderPerso(Perso* p, SDL_Renderer* renderer) {
    if (p->direction == 1) {
        SDL_RenderCopyEx(renderer, p->sprite, &p->posSprite, &p->posScreen, 0, NULL, SDL_FLIP_HORIZONTAL);
    } else {
        SDL_RenderCopy(renderer, p->sprite, &p->posSprite, &p->posScreen);
    }
}

void showHUD(Perso* p, SDL_Renderer* renderer) {
    SDL_Rect healthBack = {20, 20, 200, 20};
    SDL_Rect healthFront = {20, 20, p->hp * 2, 20};
    SDL_Rect scoreBack = {20, 50, 200, 10};
    SDL_Rect scoreFront = {20, 50, p->score % 200, 10};
    
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(renderer, &healthBack);
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthFront);
    
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(renderer, &scoreBack);
    SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
    SDL_RenderFillRect(renderer, &scoreFront);
}
