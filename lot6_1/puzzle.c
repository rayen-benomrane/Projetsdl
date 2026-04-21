#include "puzzle.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#endif

Animation animation = {0};
int soundEnabled = 1;
Mix_Chunk *placeSound   = NULL;
Mix_Chunk *victorySound = NULL;
Mix_Chunk *wrongSound   = NULL;
Mix_Music *backgroundMusic = NULL;

int draggingPiece = -1;
int dragOffsetX   = 0;
int dragOffsetY   = 0;
int wasInSideArea = 0;

int fileExists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

void createDirectories(void) {
    const char *dirs[] = {
        "assets", "assets/images", "assets/images/thumbnails",
        "assets/sounds", "assets/fonts"
    };
    for (int i = 0; i < 5; i++) {
        if (!fileExists(dirs[i])) {
            mkdir(dirs[i], 0700);
            printf("Dossier cree: %s\n", dirs[i]);
        }
    }
}

TTF_Font *loadFont(int size) {
    const char *fontPaths[] = {
        "assets/fonts/arial.ttf",
        "assets/fonts/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        NULL
    };
    for (int i = 0; fontPaths[i] != NULL; i++) {
        if (fileExists(fontPaths[i]))
            return TTF_OpenFont(fontPaths[i], size);
    }
    return NULL;
}

void playSound(Mix_Chunk *sound) {
    if (soundEnabled && sound)
        Mix_PlayChannel(-1, sound, 0);
}

Mix_Chunk *loadSound(const char *path) {
    char fullPath[256];
    sprintf(fullPath, "assets/sounds/%s", path);
    if (fileExists(fullPath))
        return Mix_LoadWAV(fullPath);
    return NULL;
}

void playMusic(const char *path) {
    char fullPath[256];
    sprintf(fullPath, "assets/sounds/%s", path);
    if (fileExists(fullPath)) {
        backgroundMusic = Mix_LoadMUS(fullPath);
        if (backgroundMusic && soundEnabled)
            Mix_PlayMusic(backgroundMusic, -1);
    }
}

SDL_Texture *loadBackground(SDL_Renderer *renderer, const char *path) {
    char fullPath[256];
    sprintf(fullPath, "assets/images/%s", path);
    if (fileExists(fullPath)) {
        SDL_Surface *surface = IMG_Load(fullPath);
        if (surface) {
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            return tex;
        }
    }
    return NULL;
}

SDL_Texture *createDefaultBackground(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 45, 255);
    SDL_RenderClear(renderer);
    return NULL;
}

SDL_Texture *loadBlackImage(SDL_Renderer *renderer, int index, int width, int height) {
    char path[256];
    sprintf(path, "assets/images/thumbnails/noir%d.jpg", index);

    if (fileExists(path)) {
        SDL_Surface *surface = IMG_Load(path);
        if (surface) {
            if (surface->w != width || surface->h != height) {
                SDL_Surface *scaled = SDL_CreateRGBSurface(
                    0, width, height, 32,
                    0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
                if (scaled) {
                    SDL_BlitScaled(surface, NULL, scaled, NULL);
                    SDL_FreeSurface(surface);
                    surface = scaled;
                }
            }
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            return tex;
        }
    }

    SDL_Texture *blackTex = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, width, height);
    if (blackTex) {
        SDL_SetRenderTarget(renderer, blackTex);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        for (int i = 0; i < width; i += 10)
            for (int j = 0; j < height; j += 10)
                SDL_RenderDrawPoint(renderer, i, j);
        SDL_SetRenderTarget(renderer, NULL);
    }
    return blackTex;
}

int scanAvailableImages(PuzzleImage *images, int maxImages) {
    int count = 0;
    printf("\n=== RECHERCHE DES IMAGES ===\n");

    const char *imgFiles[] = {
        "puzzle1.jpg","puzzle2.jpg","puzzle3.jpg","puzzle4.jpg","puzzle5.jpg"
    };
    const char *imgNames[] = {
        "Puzzle 1","Puzzle 2","Puzzle 3","Puzzle 4","Puzzle 5"
    };

    for (int i = 0; i < 5 && count < maxImages; i++) {
        char fullPath[256];
        sprintf(fullPath, "assets/images/thumbnails/%s", imgFiles[i]);
        if (fileExists(fullPath)) {
            strcpy(images[count].path, fullPath);
            strcpy(images[count].name, imgNames[i]);
            images[count].texture = NULL;
            images[count].width   = 0;
            images[count].height  = 0;
            images[count].loaded  = 0;
            printf("  Image trouvee: %s\n", imgFiles[i]);
            count++;
        } else {
            printf("  Image manquante: %s\n", imgFiles[i]);
        }
    }

    if (count == 0) {
        printf("  Aucune image — images de secours\n");
        for (int i = 0; i < 5 && i < maxImages; i++) {
            sprintf(images[count].path, "fallback_%d", i+1);
            sprintf(images[count].name, "Puzzle%d", i+1);
            images[count].texture = NULL;
            images[count].loaded  = 0;
            count++;
        }
    }
    printf("  Total : %d image(s)\n", count);
    return count;
}

SDL_Texture *loadPuzzleImage(SDL_Renderer *renderer, const char *path,
                              int *width, int *height) {
    if (strncmp(path, "fallback", 8) == 0) {
        *width = *height = PUZZLE_SIZE;
        int num = atoi(path + 9);
        if (num == 0) num = 1;

        SDL_Surface *surface = SDL_CreateRGBSurface(
            0, PUZZLE_SIZE, PUZZLE_SIZE, 32,
            0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        SDL_Color colors[] = {
            {100,100,200,255},{200,100,100,255},{100,200,100,255},
            {200,200,100,255},{200,100,200,255}
        };
        SDL_Color bg = colors[(num-1) % 5];
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, bg.r, bg.g, bg.b));

        TTF_Font *font = loadFont(60);
        if (font) {
            char numStr[5];
            sprintf(numStr, "%d", num);
            SDL_Color white = {255,255,255,255};
            SDL_Surface *ts = TTF_RenderUTF8_Blended(font, numStr, white);
            if (ts) {
                SDL_Rect dst = {(PUZZLE_SIZE-ts->w)/2, (PUZZLE_SIZE-ts->h)/2,
                                ts->w, ts->h};
                SDL_BlitSurface(ts, NULL, surface, &dst);
                SDL_FreeSurface(ts);
            }
            TTF_CloseFont(font);
        }
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        return tex;
    }

    SDL_Surface *surface = IMG_Load(path);
    if (!surface) return NULL;

    if (surface->w != PUZZLE_SIZE || surface->h != PUZZLE_SIZE) {
        SDL_Surface *scaled = SDL_CreateRGBSurface(
            0, PUZZLE_SIZE, PUZZLE_SIZE, 32,
            0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        if (scaled) {
            SDL_BlitScaled(surface, NULL, scaled, NULL);
            SDL_FreeSurface(surface);
            surface = scaled;
        }
    }
    *width = *height = PUZZLE_SIZE;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

SDL_Texture *createPieceTexture(SDL_Renderer *renderer, SDL_Texture *source,
                                 int pieceX, int pieceY, int pieceW, int pieceH) {
    SDL_Texture *pieceTex = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, pieceW, pieceH);
    if (!pieceTex) return NULL;

    SDL_SetRenderTarget(renderer, pieceTex);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_Rect src  = {pieceX, pieceY, pieceW, pieceH};
    SDL_Rect dest = {0, 0, pieceW, pieceH};
    SDL_RenderCopy(renderer, source, &src, &dest);
    SDL_SetRenderTarget(renderer, NULL);
    return pieceTex;
}

void initTargetPositions(PuzzlePiece *pieces, int gridSize,
                          int pieceSize, int puzzleX, int puzzleY) {
    for (int row = 0; row < gridSize; row++) {
        for (int col = 0; col < gridSize; col++) {
            int idx = row * gridSize + col;
            pieces[idx].targetRect.x = puzzleX + col * pieceSize;
            pieces[idx].targetRect.y = puzzleY + row * pieceSize;
            pieces[idx].targetRect.w = pieceSize;
            pieces[idx].targetRect.h = pieceSize;
        }
    }
}

int isPuzzleComplete(PuzzlePiece *pieces, int pieceCount) {
    for (int i = 0; i < pieceCount; i++) {
        if (pieces[i].originalId != i) return 0;
    }
    return 1;
}

void setupGame(PuzzlePiece *pieces, PuzzlePiece *sidePieces,
               int pieceCount, int sideCount, int pieceSize,
               SDL_Renderer *renderer, SDL_Texture *sourceImage,
               int gridSize, int puzzleX, int puzzleY) {

    printf("\n=== CONFIGURATION DU JEU ===\n");

    for (int i = 0; i < pieceCount; i++) {
        if (pieces[i].texture) { SDL_DestroyTexture(pieces[i].texture); pieces[i].texture = NULL; }
        pieces[i].texture    = createPieceTexture(renderer, sourceImage,
                                   (i % gridSize) * pieceSize,
                                   (i / gridSize) * pieceSize,
                                   pieceSize, pieceSize);
        pieces[i].rect       = pieces[i].targetRect;
        pieces[i].isInSideArea = 0;
        pieces[i].sideIndex    = -1;
        pieces[i].originalId   = i;
    }
    printf("  1. 9 pieces normales creees\n");

    int blackSlots[3];
    int chosen[9] = {0};
    int found = 0;
    while (found < sideCount) {
        int r = rand() % pieceCount;
        if (!chosen[r]) {
            chosen[r] = 1;
            blackSlots[found++] = r;
            printf("  2. Cellule %d selectionnee\n", r);
        }
    }


    for (int i = 0; i < sideCount; i++) {
        int pos = blackSlots[i];


        if (sidePieces[i].texture) { SDL_DestroyTexture(sidePieces[i].texture); sidePieces[i].texture = NULL; }
        sidePieces[i].texture    = pieces[pos].texture;
        pieces[pos].texture      = NULL;

        sidePieces[i].rect.x      = SIDE_AREA_X + 10;
        sidePieces[i].rect.y      = 80 + i * (pieceSize + 15);
        sidePieces[i].rect.w      = pieceSize;
        sidePieces[i].rect.h      = pieceSize;
        sidePieces[i].targetRect   = pieces[pos].targetRect;
        sidePieces[i].isInSideArea = 1;
        sidePieces[i].sideIndex    = i;
        sidePieces[i].originalId   = pos; 
        pieces[pos].texture    = loadBlackImage(renderer, i+1, pieceSize, pieceSize);
        pieces[pos].originalId = -1;

        printf("  3. Cellule %d -> noir%d | original -> lateral[%d]\n", pos, i+1, i);
    }


    printf("  4. Melange...\n");
    for (int i = pieceCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        SDL_Texture *tmpTex  = pieces[i].texture;
        pieces[i].texture    = pieces[j].texture;
        pieces[j].texture    = tmpTex;
        int tmpId            = pieces[i].originalId;
        pieces[i].originalId = pieces[j].originalId;
        pieces[j].originalId = tmpId;
    }
    printf("  Configuration terminee !\n");
}

int isPointInSideArea(int mx, int my) {
    return (mx >= SIDE_AREA_X && mx <= SIDE_AREA_X + SIDE_AREA_WIDTH &&
            my >= 20 && my <= WINDOW_HEIGHT - 20);
}

int getPieceIndexAtPosition(PuzzlePiece *pieces, int pieceCount,
                             PuzzlePiece *sidePieces, int sideCount,
                             int mx, int my, int *isSidePiece) {
    for (int i = 0; i < sideCount; i++) {
        if (mx >= sidePieces[i].rect.x &&
            mx <  sidePieces[i].rect.x + sidePieces[i].rect.w &&
            my >= sidePieces[i].rect.y &&
            my <  sidePieces[i].rect.y + sidePieces[i].rect.h) {
            *isSidePiece = 1;
            return i;
        }
    }
    for (int i = 0; i < pieceCount; i++) {
        if (mx >= pieces[i].rect.x &&
            mx <  pieces[i].rect.x + pieces[i].rect.w &&
            my >= pieces[i].rect.y &&
            my <  pieces[i].rect.y + pieces[i].rect.h) {
            *isSidePiece = 0;
            return i;
        }
    }
    return -1;
}

void snapToGrid(PuzzlePiece *piece, int pieceSize,
                int gridOffsetX, int gridOffsetY, int gridSize) {
    int col = (piece->rect.x - gridOffsetX + pieceSize/2) / pieceSize;
    int row = (piece->rect.y - gridOffsetY + pieceSize/2) / pieceSize;
    if (col < 0) col = 0; if (col >= gridSize) col = gridSize-1;
    if (row < 0) row = 0; if (row >= gridSize) row = gridSize-1;
    piece->rect.x = gridOffsetX + col * pieceSize;
    piece->rect.y = gridOffsetY + row * pieceSize;
}

int checkCollision(PuzzlePiece *pieces, int pieceCount, int currentIndex,
                   int pieceSize, int gridOffsetX, int gridOffsetY, int gridSize) {
    (void)pieceSize; (void)gridOffsetX; (void)gridOffsetY; (void)gridSize;
    for (int i = 0; i < pieceCount; i++) {
        if (i != currentIndex &&
            pieces[i].rect.x == pieces[currentIndex].rect.x &&
            pieces[i].rect.y == pieces[currentIndex].rect.y)
            return i;
    }
    return -1;
}

void drawCircularTimer(SDL_Renderer *renderer, int cx, int cy, int r, float pct) {
    if (pct < 0) pct = 0; if (pct > 1) pct = 1;
    SDL_SetRenderDrawColor(renderer, 60, 60, 80, 200);
    for (int w = 0; w < 8; w++)
        for (int a = 0; a < 360; a++) {
            float rad = a * (float)M_PI / 180.0f;
            SDL_RenderDrawPoint(renderer, cx+(int)((r-w)*cos(rad)), cy+(int)((r-w)*sin(rad)));
        }
    int end = (int)(360 * pct);
    SDL_Color c = pct > 0.5f ? (SDL_Color){0,200,0,255} :
                  pct > 0.25f? (SDL_Color){255,200,0,255} : (SDL_Color){200,50,0,255};
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
    for (int a = 0; a < end; a++) {
        float rad = a * (float)M_PI / 180.0f;
        for (int w = 0; w < 6; w++)
            SDL_RenderDrawPoint(renderer, cx+(int)((r-w)*cos(rad)), cy+(int)((r-w)*sin(rad)));
    }
    if (pct < 0.25f) {
        int alpha = 100 + (int)(155 * sin(SDL_GetTicks() * 0.02));
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, alpha);
        for (int a = 0; a < 360; a += 5) {
            float rad = a * (float)M_PI / 180.0f;
            SDL_RenderDrawPoint(renderer, cx+(int)((r+5)*cos(rad)), cy+(int)((r+5)*sin(rad)));
        }
    }
}

void drawLinearTimerBar(SDL_Renderer *renderer, int x, int y, int w, int h, float pct) {
    if (pct < 0) pct = 0; if (pct > 1) pct = 1;
    SDL_SetRenderDrawColor(renderer, 40, 40, 60, 200);
    SDL_Rect bg = {x, y, w, h}; SDL_RenderFillRect(renderer, &bg);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150); SDL_RenderDrawRect(renderer, &bg);
    SDL_Color c = pct > 0.5f ? (SDL_Color){0,200,0,255} :
                  pct > 0.25f? (SDL_Color){255,200,0,255} : (SDL_Color){200,50,0,255};
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
    SDL_Rect fill = {x, y, (int)(w * pct), h}; SDL_RenderFillRect(renderer, &fill);
}

void drawTimerAnimation(SDL_Renderer *renderer, float timeRemaining, float timeLimit) {
    float pct = timeRemaining / timeLimit;
    if (pct < 0) pct = 0; if (pct > 1) pct = 1;
    drawLinearTimerBar(renderer, (WINDOW_WIDTH-400)/2, 15, 400, 12, pct);
    drawCircularTimer(renderer, WINDOW_WIDTH-55, 55, 35, pct);
    Uint32 t = SDL_GetTicks();
    int sa = (int)((t/20) % 360);
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 180);
    for (int i = 0; i < 4; i++) {
        float rad = (sa + i*90) * (float)M_PI / 180.0f;
        SDL_RenderDrawPoint(renderer, WINDOW_WIDTH-55+(int)(27*cos(rad)), 55+(int)(27*sin(rad)));
    }
}

void drawSideArea(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 70, 220);
    SDL_Rect area = {SIDE_AREA_X, 20, SIDE_AREA_WIDTH, WINDOW_HEIGHT-40};
    SDL_RenderFillRect(renderer, &area);
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 180);
    SDL_RenderDrawRect(renderer, &area);
    if (font) {
        SDL_Color white = {255,255,255,255};
        SDL_Surface *ts = TTF_RenderUTF8_Blended(font, "PIECES A REMETTRE", white);
        if (ts) {
            SDL_Texture *tt = SDL_CreateTextureFromSurface(renderer, ts);
            SDL_Rect r = {SIDE_AREA_X+(SIDE_AREA_WIDTH-ts->w)/2, 30, ts->w, ts->h};
            SDL_RenderCopy(renderer, tt, NULL, &r);
            SDL_DestroyTexture(tt); SDL_FreeSurface(ts);
        }
    }
}

void drawSelectedPieceHighlight(SDL_Renderer *renderer, SDL_Rect rect) {
    int alpha = 180 + (int)(75 * sin(SDL_GetTicks() * 0.01));
    if (alpha > 255) alpha = 255;
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, alpha);
    SDL_Rect h = {rect.x-4, rect.y-4, rect.w+8, rect.h+8};
    for (int i = 0; i < 4; i++) {
        SDL_RenderDrawRect(renderer, &h);
        h.x++; h.y++; h.w -= 2; h.h -= 2;
    }
}

static void renderText(SDL_Renderer *renderer, TTF_Font *font,
                        const char *text, SDL_Color color, int cx, int cy) {
    if (!font) return;
    SDL_Surface *ts = TTF_RenderUTF8_Blended(font, text, color);
    if (!ts) return;
    SDL_Texture *tt = SDL_CreateTextureFromSurface(renderer, ts);
    SDL_Rect r = {cx - ts->w/2, cy - ts->h/2, ts->w, ts->h};
    SDL_RenderCopy(renderer, tt, NULL, &r);
    SDL_DestroyTexture(tt); SDL_FreeSurface(ts);
}

void puzzleGame(SDL_Window *window, SDL_Renderer *renderer) {
    (void)window;

    printf("\n=== INITIALISATION ===\n");
    createDirectories();
    srand((unsigned)time(NULL));

    TTF_Font *regularFont = loadFont(22);
    TTF_Font *smallFont   = loadFont(14);

    placeSound   = loadSound("place.wav");
    victorySound = loadSound("victory.wav");
    wrongSound   = loadSound("wrong.wav");
    playMusic("music.mp3");

    AssetManager assets = {0};
    assets.backgroundTexture = loadBackground(renderer, "background.png");
    if (!assets.backgroundTexture) createDefaultBackground(renderer);
    assets.gameBackgroundTexture = loadBackground(renderer, "game_background.png");

    assets.imageCount = scanAvailableImages(assets.images, MAX_IMAGES);
    for (int i = 0; i < assets.imageCount; i++) {
        assets.images[i].texture = loadPuzzleImage(renderer, assets.images[i].path,
                                                    &assets.images[i].width,
                                                    &assets.images[i].height);
        if (assets.images[i].texture) {
            assets.images[i].loaded = 1;
            printf("Chargee : %s\n", assets.images[i].name);
        }
    }

    int selectedImage = (assets.imageCount > 0) ? rand() % assets.imageCount : 0;
    printf("Image selectionnee : %s\n", assets.images[selectedImage].name);

    int imageWidth = 0, imageHeight = 0;
    SDL_Texture *currentImage = NULL;
    if (selectedImage < assets.imageCount && assets.images[selectedImage].loaded)
        currentImage = assets.images[selectedImage].texture;
    if (!currentImage)
        currentImage = loadPuzzleImage(renderer, "fallback_1", &imageWidth, &imageHeight);

    const int gridSize   = 3;
    const int pieceSize  = PUZZLE_SIZE / gridSize;
    const int puzzleX    = (WINDOW_WIDTH - PUZZLE_SIZE - SIDE_AREA_WIDTH - 30) / 2;
    const int puzzleY    = 60;
    const int pieceCount = 9;
    const int sideCount  = 3;

    PuzzlePiece *pieces = calloc(pieceCount, sizeof(PuzzlePiece));
    for (int i = 0; i < pieceCount; i++) {
        pieces[i].id = i; pieces[i].rect.w = pieces[i].rect.h = pieceSize;
        pieces[i].originalId = i; pieces[i].isInSideArea = 0; pieces[i].sideIndex = -1;
    }
    initTargetPositions(pieces, gridSize, pieceSize, puzzleX, puzzleY);

    PuzzlePiece *sidePieces = calloc(sideCount, sizeof(PuzzlePiece));
    for (int i = 0; i < sideCount; i++) {
        sidePieces[i].id = i; sidePieces[i].rect.w = sidePieces[i].rect.h = pieceSize;
        sidePieces[i].isInSideArea = 1; sidePieces[i].sideIndex = i; sidePieces[i].originalId = -1;
    }

    setupGame(pieces, sidePieces, pieceCount, sideCount, pieceSize,
              renderer, currentImage, gridSize, puzzleX, puzzleY);

    int running = 1, victory = 0, gameOver = 0;
    Uint32 gameStartTime = SDL_GetTicks();
    Uint32 victoryTime   = 0;
    float timeRemaining = TIME_LIMIT;
    draggingPiece = -1;

    SDL_Color white = {255,255,255,255};
    SDL_Color gold  = {255,215,0,255};

    printf("\nTouches: R=Recommencer | ESPACE=Nouvelle image | M=Son | ESC=Quitter\n\n");

    while (running) {


        if (!victory && !gameOver) {
            float elapsed = (SDL_GetTicks() - gameStartTime) / 1000.0f;
            timeRemaining = TIME_LIMIT - elapsed;
            if (timeRemaining <= 0.0f) {
                timeRemaining = 0.0f; gameOver = 1;
                playSound(wrongSound);
                printf("TEMPS ECOULE !\n");
            }
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { running = 0; break; }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: running = 0; break;
                    case SDLK_m:
                        soundEnabled = !soundEnabled;
                        if (!soundEnabled) { Mix_HaltChannel(-1); Mix_HaltMusic(); }
                        else if (backgroundMusic) Mix_PlayMusic(backgroundMusic, -1);
                        break;
                    case SDLK_r:
                        victory = 0; gameOver = 0; draggingPiece = -1;
                        victoryTime = 0;
                        gameStartTime = SDL_GetTicks(); timeRemaining = TIME_LIMIT;
                        setupGame(pieces, sidePieces, pieceCount, sideCount, pieceSize,
                                  renderer, currentImage, gridSize, puzzleX, puzzleY);
                        printf("Recommence !\n"); break;
                    case SDLK_SPACE:
                        if (!victory && !gameOver && assets.imageCount > 1) {
                            int nImg;
                            do { nImg = rand() % assets.imageCount; } while (nImg == selectedImage);
                            if (assets.images[nImg].loaded) {
                                selectedImage = nImg;
                                currentImage  = assets.images[selectedImage].texture;
                                victory = 0; gameOver = 0; draggingPiece = -1;
                                victoryTime = 0;
                                gameStartTime = SDL_GetTicks(); timeRemaining = TIME_LIMIT;
                                setupGame(pieces, sidePieces, pieceCount, sideCount, pieceSize,
                                          renderer, currentImage, gridSize, puzzleX, puzzleY);
                                printf("Nouvelle image : %s\n", assets.images[selectedImage].name);
                            }
                        }
                        break;
                    default: break;
                }
            }

            if (!victory && !gameOver) {


                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                    int mx = event.button.x, my = event.button.y;
                    int isSide = 0;
                    int idx = getPieceIndexAtPosition(pieces, pieceCount,
                                                      sidePieces, sideCount, mx, my, &isSide);
                    if (idx != -1) {
                        draggingPiece = idx; wasInSideArea = isSide;
                        if (isSide) {
                            dragOffsetX = mx - sidePieces[idx].rect.x;
                            dragOffsetY = my - sidePieces[idx].rect.y;
                        } else {
                            dragOffsetX = mx - pieces[idx].rect.x;
                            dragOffsetY = my - pieces[idx].rect.y;
                        }
                    }
                }


                if (event.type == SDL_MOUSEMOTION && draggingPiece != -1) {
                    if (wasInSideArea) {
                        sidePieces[draggingPiece].rect.x = event.motion.x - dragOffsetX;
                        sidePieces[draggingPiece].rect.y = event.motion.y - dragOffsetY;
                    } else {
                        pieces[draggingPiece].rect.x = event.motion.x - dragOffsetX;
                        pieces[draggingPiece].rect.y = event.motion.y - dragOffsetY;
                    }
                }


                if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT
                    && draggingPiece != -1) {

                    int mx = event.button.x, my = event.button.y;
                    int targetIdx = -1, targetSide = 0;
                    for (int i = 0; i < pieceCount; i++) {
                        if (!wasInSideArea && i == draggingPiece) continue;
                        if (mx >= pieces[i].targetRect.x &&
                            mx <  pieces[i].targetRect.x + pieceSize &&
                            my >= pieces[i].targetRect.y &&
                            my <  pieces[i].targetRect.y + pieceSize) {

                            if (!wasInSideArea && i == draggingPiece) continue;
                            targetIdx = i; targetSide = 0; break;
                        }
                    }


                    if (targetIdx == -1) {
                        for (int i = 0; i < sideCount; i++) {
                            if (wasInSideArea && i == draggingPiece) continue;
                            if (mx >= sidePieces[i].rect.x &&
                                mx <  sidePieces[i].rect.x + sidePieces[i].rect.w &&
                                my >= sidePieces[i].rect.y &&
                                my <  sidePieces[i].rect.y + sidePieces[i].rect.h) {
                                targetIdx = i; targetSide = 1; break;
                            }
                        }
                    }

                    if (targetIdx != -1) {

#define SWAP_TEX(a, b)  do { SDL_Texture *_t=(a);(a)=(b);(b)=_t; } while(0)
#define SWAP_INT(a, b)  do { int _i=(a);(a)=(b);(b)=_i; } while(0)

                        if (!wasInSideArea && !targetSide) {

                            SWAP_TEX(pieces[draggingPiece].texture, pieces[targetIdx].texture);
                            SWAP_INT(pieces[draggingPiece].originalId, pieces[targetIdx].originalId);
                            pieces[draggingPiece].rect = pieces[draggingPiece].targetRect;
                            pieces[targetIdx].rect     = pieces[targetIdx].targetRect;
                            playSound(placeSound);
                            printf("Echange grille[%d] <-> grille[%d]  (ids: %d, %d)\n",
                                   draggingPiece, targetIdx,
                                   pieces[draggingPiece].originalId, pieces[targetIdx].originalId);
                        }
                        else if (wasInSideArea && targetSide) {

                            SWAP_TEX(sidePieces[draggingPiece].texture, sidePieces[targetIdx].texture);
                            SWAP_INT(sidePieces[draggingPiece].originalId, sidePieces[targetIdx].originalId);
                            sidePieces[draggingPiece].rect.x = SIDE_AREA_X + 10;
                            sidePieces[draggingPiece].rect.y = 80 + draggingPiece * (pieceSize+15);
                            sidePieces[targetIdx].rect.x     = SIDE_AREA_X + 10;
                            sidePieces[targetIdx].rect.y     = 80 + targetIdx * (pieceSize+15);
                            playSound(placeSound);
                            printf("Echange lateral[%d] <-> lateral[%d]\n", draggingPiece, targetIdx);
                        }
                        else if (!wasInSideArea && targetSide) {

                            SWAP_TEX(pieces[draggingPiece].texture, sidePieces[targetIdx].texture);
                            SWAP_INT(pieces[draggingPiece].originalId, sidePieces[targetIdx].originalId);
                            pieces[draggingPiece].rect   = pieces[draggingPiece].targetRect;
                            sidePieces[targetIdx].rect.x = SIDE_AREA_X + 10;
                            sidePieces[targetIdx].rect.y = 80 + targetIdx * (pieceSize+15);
                            playSound(placeSound);
                            printf("Echange grille[%d] <-> lateral[%d]\n", draggingPiece, targetIdx);
                        }
                        else { 
                            SWAP_TEX(sidePieces[draggingPiece].texture, pieces[targetIdx].texture);
                            SWAP_INT(sidePieces[draggingPiece].originalId, pieces[targetIdx].originalId);
                            sidePieces[draggingPiece].rect.x = SIDE_AREA_X + 10;
                            sidePieces[draggingPiece].rect.y = 80 + draggingPiece * (pieceSize+15);
                            pieces[targetIdx].rect = pieces[targetIdx].targetRect;
                            playSound(placeSound);
                            printf("Echange lateral[%d] <-> grille[%d]\n", draggingPiece, targetIdx);
                        }


                        if (isPuzzleComplete(pieces, pieceCount)) {
                            victory = 1;
                            victoryTime = SDL_GetTicks();
                            playSound(victorySound);
                            printf("\n*** VICTOIRE ! Toutes les pieces sont en place ! ***\n\n");
                        }

                    } else {

                        if (wasInSideArea) {
                            sidePieces[draggingPiece].rect.x = SIDE_AREA_X + 10;
                            sidePieces[draggingPiece].rect.y = 80 + draggingPiece * (pieceSize+15);
                        } else {
                            pieces[draggingPiece].rect = pieces[draggingPiece].targetRect;
                        }
                    }

                    draggingPiece = -1;
                }
            }
        }

      
        if (assets.gameBackgroundTexture)
            SDL_RenderCopy(renderer, assets.gameBackgroundTexture, NULL, NULL);
        else if (assets.backgroundTexture)
            SDL_RenderCopy(renderer, assets.backgroundTexture, NULL, NULL);
        else { SDL_SetRenderDrawColor(renderer, 25, 25, 40, 255); SDL_RenderClear(renderer); }

        drawTimerAnimation(renderer, timeRemaining, TIME_LIMIT);
        drawSideArea(renderer, smallFont);


        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 180);
        SDL_RenderDrawRect(renderer, &(SDL_Rect){puzzleX-3, puzzleY-3, PUZZLE_SIZE+6, PUZZLE_SIZE+6});
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(renderer, &(SDL_Rect){puzzleX, puzzleY, PUZZLE_SIZE, PUZZLE_SIZE});


        SDL_SetRenderDrawColor(renderer, 100, 100, 130, 200);
        for (int i = 0; i <= gridSize; i++) {
            SDL_RenderDrawLine(renderer, puzzleX+i*pieceSize, puzzleY, puzzleX+i*pieceSize, puzzleY+PUZZLE_SIZE);
            SDL_RenderDrawLine(renderer, puzzleX, puzzleY+i*pieceSize, puzzleX+PUZZLE_SIZE, puzzleY+i*pieceSize);
        }


        if (smallFont) {
            char title[128];
            sprintf(title, "PUZZLE - %s", assets.images[selectedImage].name);
            SDL_Surface *ts = TTF_RenderUTF8_Blended(smallFont, title, gold);
            if (ts) {
                SDL_Texture *tt = SDL_CreateTextureFromSurface(renderer, ts);
                SDL_RenderCopy(renderer, tt, NULL, &(SDL_Rect){20,15,ts->w,ts->h});
                SDL_DestroyTexture(tt); SDL_FreeSurface(ts);
            }
        }


        for (int i = 0; i < pieceCount; i++) {
            if (i == draggingPiece && !wasInSideArea) continue;
            if (pieces[i].texture)
                SDL_RenderCopy(renderer, pieces[i].texture, NULL, &pieces[i].rect);
        }


        for (int i = 0; i < sideCount; i++) {
            if (i == draggingPiece && wasInSideArea) continue;
            if (sidePieces[i].texture)
                SDL_RenderCopy(renderer, sidePieces[i].texture, NULL, &sidePieces[i].rect);
        }


        if (draggingPiece != -1) {
            SDL_Rect *dr; SDL_Texture *dt;
            if (wasInSideArea) { dr = &sidePieces[draggingPiece].rect; dt = sidePieces[draggingPiece].texture; }
            else               { dr = &pieces[draggingPiece].rect;     dt = pieces[draggingPiece].texture; }
            if (dt) SDL_RenderCopy(renderer, dt, NULL, dr);
            drawSelectedPieceHighlight(renderer, *dr);
        }


        if (smallFont) {
            SDL_Surface *ts = TTF_RenderUTF8_Blended(smallFont,
                "Glissez et deposez | R=Rejouer | ESPACE=Nouvelle image | ESC=Quitter", white);
            if (ts) {
                SDL_Texture *tt = SDL_CreateTextureFromSurface(renderer, ts);
                SDL_Rect r = {WINDOW_WIDTH/2-ts->w/2, WINDOW_HEIGHT-22, ts->w, ts->h};
                SDL_RenderCopy(renderer, tt, NULL, &r);
                SDL_DestroyTexture(tt); SDL_FreeSurface(ts);
            }
        }

        if (victory) {
            Uint32 now = SDL_GetTicks();


            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
            SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT});


            int panX = 80, panY = 100, panW = WINDOW_WIDTH - 160, panH = 330;


            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
            SDL_RenderFillRect(renderer, &(SDL_Rect){panX+6, panY+6, panW, panH});


            SDL_SetRenderDrawColor(renderer, 10, 40, 15, 245);
            SDL_RenderFillRect(renderer, &(SDL_Rect){panX, panY, panW, panH});


            int bAlpha = 180 + (int)(75 * sin(now * 0.004));
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, bAlpha);
            for (int b = 0; b < 4; b++)
                SDL_RenderDrawRect(renderer, &(SDL_Rect){panX-b, panY-b, panW+b*2, panH+b*2});


            SDL_SetRenderDrawColor(renderer, 0, 200, 80, 200);
            SDL_RenderDrawRect(renderer, &(SDL_Rect){panX+6, panY+6, panW-12, panH-12});


            int starCenters[4][2] = {
                {panX+30,  panY+30},
                {panX+panW-30, panY+30},
                {panX+30,  panY+panH-30},
                {panX+panW-30, panY+panH-30}
            };
            for (int s = 0; s < 4; s++) {
                int sx = starCenters[s][0], sy = starCenters[s][1];
                float starAngle = now * 0.003f + s * 1.5708f;
                for (int ray = 0; ray < 8; ray++) {
                    float a = starAngle + ray * (float)M_PI / 4.0f;
                    int len = (ray % 2 == 0) ? 16 : 8;
                    int ex  = sx + (int)(len * cos(a));
                    int ey  = sy + (int)(len * sin(a));
                    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 230);
                    SDL_RenderDrawLine(renderer, sx, sy, ex, ey);
                }
                SDL_SetRenderDrawColor(renderer, 255, 255, 180, 255);
                SDL_RenderFillRect(renderer, &(SDL_Rect){sx-3, sy-3, 6, 6});
            }


            SDL_Color confettiColors[] = {
                {255,215,0,255}, {0,220,80,255}, {80,180,255,255},
                {255,100,180,255}, {255,160,50,255}
            };
            for (int p = 0; p < 30; p++) {

                float px = (float)((p * 137 + 50) % (WINDOW_WIDTH - 20));
                float speed = 40.0f + (p % 5) * 15.0f;
                float t_sec = (now - victoryTime) / 1000.0f;
                float py  = fmodf(panY + 20.0f + p * 7.0f + t_sec * speed, (float)(panH - 20));
                SDL_Color cc = confettiColors[p % 5];
                SDL_SetRenderDrawColor(renderer, cc.r, cc.g, cc.b, 200);
                int pw = 4 + (p % 3) * 2, ph = 4 + (p % 4);
                SDL_RenderFillRect(renderer, &(SDL_Rect){(int)px, (int)py, pw, ph});
            }


            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 160);
            SDL_RenderDrawLine(renderer, panX+20, panY+50, panX+panW-20, panY+50);


            if (regularFont) {

                int haloAlpha = 120 + (int)(80 * sin(now * 0.006));
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, haloAlpha);
                for (int h = 0; h < 3; h++)
                    SDL_RenderDrawRect(renderer, &(SDL_Rect){
                        WINDOW_WIDTH/2 - 200 - h,
                        panY + 58 - h,
                        400 + h*2, 40 + h*2
                    });
                renderText(renderer, regularFont,
                           "*** FELICITATIONS ! ***",
                           gold, WINDOW_WIDTH/2, panY + 78);
            }


            if (regularFont) {
                SDL_Color green = {80, 255, 120, 255};
                renderText(renderer, regularFont,
                           "PUZZLE RESOLU !",
                           green, WINDOW_WIDTH/2, panY + 130);
            }


            SDL_SetRenderDrawColor(renderer, 0, 200, 80, 140);
            SDL_RenderDrawLine(renderer, panX+40, panY+155, panX+panW-40, panY+155);


            if (smallFont) {

                int score = (int)(timeRemaining * 100);
                char scoreStr[128];
                sprintf(scoreStr, "Score :  %d  |  Temps restant :  %.0f s", score, timeRemaining);
                SDL_Color cyan = {100, 230, 255, 255};
                renderText(renderer, smallFont, scoreStr, cyan, WINDOW_WIDTH/2, panY + 180);
            }


            if (smallFont) {
                SDL_Color lightGreen = {180, 255, 180, 255};
                renderText(renderer, smallFont,
                           "Bravo ! Toutes les pieces sont correctement placees !",
                           lightGreen, WINDOW_WIDTH/2, panY + 215);
            }



        }
        if (gameOver && !victory) {
            Uint32 now = SDL_GetTicks();


            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
            SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT});


            int panX = 100, panY = 140, panW = WINDOW_WIDTH-200, panH = 260;
            SDL_SetRenderDrawColor(renderer, 40, 5, 5, 245);
            SDL_RenderFillRect(renderer, &(SDL_Rect){panX, panY, panW, panH});


            int rAlpha = 150 + (int)(105 * sin(now * 0.008));
            SDL_SetRenderDrawColor(renderer, 220, 30, 30, rAlpha);
            for (int b = 0; b < 4; b++)
                SDL_RenderDrawRect(renderer, &(SDL_Rect){panX-b, panY-b, panW+b*2, panH+b*2});

            if (regularFont) {
                SDL_Color red = {255, 80, 80, 255};
                renderText(renderer, regularFont, "TEMPS ECOULE !", red, WINDOW_WIDTH/2, panY+70);
            }
            if (smallFont) {
                SDL_Color orange = {255, 160, 60, 255};
                renderText(renderer, smallFont,
                           "Le puzzle n'est pas termine. Reessayez !",
                           orange, WINDOW_WIDTH/2, panY+120);

                SDL_SetRenderDrawColor(renderer, 180, 30, 30, 120);
                SDL_RenderDrawLine(renderer, panX+20, panY+145, panX+panW-20, panY+145);

                renderText(renderer, smallFont,
                           "R = Recommencer  |  ESPACE = Nouvelle image  |  ESC = Quitter",
                           white, WINDOW_WIDTH/2, panY+175);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }


    for (int i = 0; i < pieceCount; i++) if (pieces[i].texture) SDL_DestroyTexture(pieces[i].texture);
    free(pieces);
    for (int i = 0; i < sideCount; i++) if (sidePieces[i].texture) SDL_DestroyTexture(sidePieces[i].texture);
    free(sidePieces);
    for (int i = 0; i < assets.imageCount; i++) if (assets.images[i].texture) SDL_DestroyTexture(assets.images[i].texture);
    if (assets.backgroundTexture)     SDL_DestroyTexture(assets.backgroundTexture);
    if (assets.gameBackgroundTexture) SDL_DestroyTexture(assets.gameBackgroundTexture);
    if (regularFont)    TTF_CloseFont(regularFont);
    if (smallFont)      TTF_CloseFont(smallFont);
    if (placeSound)     Mix_FreeChunk(placeSound);
    if (victorySound)   Mix_FreeChunk(victorySound);
    if (wrongSound)     Mix_FreeChunk(wrongSound);
    if (backgroundMusic) Mix_FreeMusic(backgroundMusic);

    printf("\n=== JEU TERMINE ===\n");
}
