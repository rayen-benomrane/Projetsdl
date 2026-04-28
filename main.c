#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "perso.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    SDL_Window* window = SDL_CreateWindow("Blood Contract", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    Perso john;
    initPerso(&john, renderer);
    int running = 1;
    SDL_Event event;
    Uint32 last_time = SDL_GetTicks();
    Uint32 anim_timer = 0;
    while (running) {
        Uint32 current_time = SDL_GetTicks();
        Uint32 dt = current_time - last_time;
        last_time = current_time;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            handlePersoInput(&john, event);
        }
        updatePersoPhysics(&john, dt);
        anim_timer += dt;
        if (anim_timer > 100) {
            animatePerso(&john);
            anim_timer = 0;
        }
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);
        renderPerso(&john, renderer);
        showHUD(&john, renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / FPS);
    }
    SDL_DestroyTexture(john.sprite);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
