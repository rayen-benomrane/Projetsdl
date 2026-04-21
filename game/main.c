#include "lot.h"

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL; SDL_Renderer* renderer = NULL; GameAssets assets;
    Player p1 = {50, 500, 0, 0, 0}; Player p2 = {50, 500, 0, 0, 0};
    int running = 1, isMulti = 0, winner = 0, finalTime = 0;
    if (!init_game(&window, &renderer, &assets)) return 1;
    Uint32 start = SDL_GetTicks();
    while (running) {
        SDL_Event e; const Uint8* ks = SDL_GetKeyboardState(NULL);
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_m) isMulti = !isMulti;
                if (e.key.keysym.sym == SDLK_SPACE && p1.isGrounded) { p1.velY = -15.0f; p1.isGrounded = 0; }
                if (e.key.keysym.sym == SDLK_UP && p2.isGrounded) { p2.velY = -15.0f; p2.isGrounded = 0; }
            }
        }
        p1.velX = ks[SDL_SCANCODE_D] ? 7.0f : (ks[SDL_SCANCODE_A] ? -7.0f : 0);
        p2.velX = ks[SDL_SCANCODE_RIGHT] ? 7.0f : (ks[SDL_SCANCODE_LEFT] ? -7.0f : 0);
        if (winner == 0) {
            update_player(&p1, &assets); if (isMulti) update_player(&p2, &assets);
            if (p1.x > 2450) { winner = 1; finalTime = (SDL_GetTicks()-start)/1000; }
            if (p2.x > 2450) { winner = 2; finalTime = (SDL_GetTicks()-start)/1000; }
        }
        render_game(renderer, &p1, &p2, &assets, isMulti, winner, (winner == 0) ? (SDL_GetTicks()-start)/1000 : finalTime);
        SDL_Delay(16);
    }
    free_assets(&assets); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
    return 0;
}
