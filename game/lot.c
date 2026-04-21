#include "lot.h"

int init_game(SDL_Window** w, SDL_Renderer** r, GameAssets* assets) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    *w = SDL_CreateWindow("Cyberpunk Game", 100, 100, 1280, 720, 0);
    *r = SDL_CreateRenderer(*w, -1, SDL_RENDERER_ACCELERATED);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    assets->bg1 = IMG_LoadTexture(*r, "bg1.jpeg");
    assets->bg2 = IMG_LoadTexture(*r, "bg2.jpeg");
    assets->obsTex = IMG_LoadTexture(*r, "obstacle.jpg");
    assets->obsRects[0] = (SDL_Rect){600, 530, 150, 100};
    assets->obsRects[1] = (SDL_Rect){1400, 530, 150, 100};
    assets->obsRects[2] = (SDL_Rect){2200, 530, 150, 100};
    return (assets->bg1 && assets->bg2 && assets->obsTex);
}

void update_player(Player* p, GameAssets* assets) {
    p->velY += 0.6f;
    p->x += p->velX;
    p->y += p->velY;
    p->scrollX = p->x - 300;
    if (p->scrollX < 0) p->scrollX = 0;
    if (p->scrollX > 1280) p->scrollX = 1280; // Stop scrolling 3and e5er bg2
    if (p->y > 580) { p->y = 580; p->velY = 0; p->isGrounded = 1; }
    if (p->x < 0) p->x = 0;
    if (p->x > 2515) p->x = 2515;
    SDL_Rect pr = {(int)p->x, (int)p->y, 45, 70};
    for(int i=0; i<3; i++) {
        if (SDL_HasIntersection(&pr, &assets->obsRects[i])) {
            if (p->velY > 0 && p->y + 70 < assets->obsRects[i].y + 30) {
                p->y = assets->obsRects[i].y - 70; p->velY = 0; p->isGrounded = 1;
            } else {
                if (p->velX > 0) p->x = assets->obsRects[i].x - 45;
                if (p->velX < 0) p->x = assets->obsRects[i].x + assets->obsRects[i].w;
            }
        }
    }
}

void draw_num(SDL_Renderer* r, int n, int x, int y) {
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    int segs[10][7] = {{1,1,1,0,1,1,1},{0,0,1,0,0,1,0},{1,0,1,1,1,0,1},{1,0,1,1,0,1,1},{0,1,1,1,0,1,0},
                       {1,1,0,1,0,1,1},{1,1,0,1,1,1,1},{1,0,1,0,0,1,0},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}};
    SDL_Rect s[7] = {{x,y,20,5},{x,y,5,20},{x+20,y,5,20},{x,y+20,20,5},{x,y+20,5,20},{x+20,y+20,5,20},{x,y+40,20,5}};
    for(int i=0; i<7; i++) if(segs[n][i]) SDL_RenderFillRect(r, &s[i]);
}

void draw_win_box(SDL_Renderer* r, int time) {
    SDL_Rect b = { 440, 240, 400, 240 };
    SDL_SetRenderDrawColor(r, 20, 20, 20, 255); SDL_RenderFillRect(r, &b);
    SDL_SetRenderDrawColor(r, 255, 215, 0, 255); SDL_RenderDrawRect(r, &b);
    SDL_SetRenderDrawColor(r, 0, 255, 255, 255);
    SDL_Rect you[] = {{480,280,10,30},{500,280,10,30},{490,310,10,20}, {520,280,30,10},{520,320,30,10},{520,280,10,50},{540,280,10,50}, {560,280,10,50},{580,280,10,50},{560,320,30,10}};
    SDL_Rect win[] = {{620,280,10,50},{650,280,10,50},{635,310,10,20}, {680,280,10,50}, {710,280,10,50},{740,280,10,50},{710,280,30,10}};
    for(int i=0; i<10; i++) SDL_RenderFillRect(r, &you[i]);
    for(int i=0; i<7; i++) SDL_RenderFillRect(r, &win[i]);
    draw_num(r, (time%60)/10, 610, 380); draw_num(r, (time%60)%10, 640, 380);
}

void render_game(SDL_Renderer* r, Player* p1, Player* p2, GameAssets* assets, int multi, int winner, int time) {
    SDL_RenderClear(r);
    int w = multi ? 640 : 1280;
    Player* ps[2] = {p1, p2};
    for(int i=0; i < (multi ? 2 : 1); i++) {
        SDL_Rect v = {i * w, 0, w, 720};
        SDL_RenderSetViewport(r, &v);
        SDL_Rect r1 = {-(int)ps[i]->scrollX / (multi?2:1), 0, 1280 / (multi?2:1), 720};
        SDL_RenderCopy(r, assets->bg1, NULL, &r1);
        SDL_Rect r2 = {(1280 - (int)ps[i]->scrollX) / (multi?2:1), 0, 1280 / (multi?2:1), 720};
        SDL_RenderCopy(r, assets->bg2, NULL, &r2);
        for(int j=0; j<3; j++) {
            SDL_Rect dO = { (assets->obsRects[j].x - (int)ps[i]->scrollX) / (multi?2:1), assets->obsRects[j].y, assets->obsRects[j].w / (multi?2:1), assets->obsRects[j].h };
            SDL_RenderCopy(r, assets->obsTex, NULL, &dO);
        }
        SDL_Rect pr = {(int)((ps[i]->x - ps[i]->scrollX) / (multi?2:1)), (int)ps[i]->y, 45 / (multi?2:1), 70};
        SDL_SetRenderDrawColor(r, (i==0?0:255), 255, (i==0?255:0), 255);
        SDL_RenderFillRect(r, &pr);
    }
    SDL_RenderSetViewport(r, NULL);
    draw_num(r, (time/60)/10, 600, 20); draw_num(r, (time/60)%10, 630, 20);
    draw_num(r, (time%60)/10, 670, 20); draw_num(r, (time%60)%10, 700, 20);
    if (winner != 0) draw_win_box(r, time);
    SDL_RenderPresent(r);
}

void free_assets(GameAssets* assets) {
    SDL_DestroyTexture(assets->bg1); SDL_DestroyTexture(assets->bg2); SDL_DestroyTexture(assets->obsTex);
}
