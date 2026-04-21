#ifndef PUZZLE_H
#define PUZZLE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 550
#define MAX_IMAGES 50
#define PUZZLE_SIZE 360
#define SIDE_AREA_WIDTH 200
#define SIDE_AREA_X (WINDOW_WIDTH - SIDE_AREA_WIDTH - 20)
#define TIME_LIMIT 60

typedef struct PuzzlePiece {
    SDL_Rect rect;
    SDL_Rect targetRect;
    SDL_Texture *texture;
    int id;
    int isInSideArea;
    int sideIndex;
    int originalId;
} PuzzlePiece;

typedef struct {
    char path[256];
    SDL_Texture *texture;
    int width;
    int height;
    int loaded;
    char name[50];
} PuzzleImage;

typedef struct {
    PuzzleImage images[MAX_IMAGES];
    int imageCount;
    SDL_Texture *backgroundTexture;
    SDL_Texture *gameBackgroundTexture;
} AssetManager;

typedef struct {
    int active;
    int pieceIndex;
    float angle;
    float scale;
    Uint32 startTime;
    int startX, startY;
    int endX, endY;
} Animation;

void puzzleGame(SDL_Window *window, SDL_Renderer *renderer);

#endif
