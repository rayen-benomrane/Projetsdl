#ifndef LOT_H
#define LOT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct {
    float x, y, velX, velY;
    float scrollX;
    int isGrounded;
} Player;

typedef struct {
    SDL_Texture *bg1, *bg2, *obsTex;
    SDL_Rect obsRects[3];
} GameAssets;

int init_game(SDL_Window** w, SDL_Renderer** r, GameAssets* assets);
void update_player(Player* p, GameAssets* assets);
void draw_num(SDL_Renderer* r, int n, int x, int y);
void render_game(SDL_Renderer* r, Player* p1, Player* p2, GameAssets* assets, int multi, int winner, int time);
void draw_win_box(SDL_Renderer* r, int time);
void free_assets(GameAssets* assets);

#endif
