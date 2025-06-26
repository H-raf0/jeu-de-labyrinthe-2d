#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NUM_BACKGROUNDS 6

// Chargement d'une texture à partir d'un fichier
SDL_Texture* LoadTexture(const char* file, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(file);
    if (!surface) {
        printf("Erreur lors du chargement de %s : %s\n", file, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

// Affiche le sprite du chevalier à une position donnée avec un index de frame
void CreateKnight(SDL_Texture* texture, SDL_Window* window, SDL_Renderer* renderer, int posX, int posY, int frameIndex) {
    SDL_Rect src = {0}, dst = {0}, win_dim = {0};
    SDL_GetWindowSize(window, &win_dim.w, &win_dim.h);
    SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);

    int columns = 8;           // 8 colonnes dans le sprite
    float zoom = 2.0f;
    int frameW = src.w / columns;
    int frameH = src.h;

    src.x = frameIndex * frameW;
    src.y = 0;
    src.w = frameW;
    src.h = frameH;

    dst.w = frameW * zoom;
    dst.h = frameH * zoom;
    dst.x = posX;
    dst.y = posY;

    SDL_RenderCopy(renderer, texture, &src, &dst);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Parallax Knight", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Charger les backgrounds
    SDL_Texture* backgrounds[NUM_BACKGROUNDS];
    float scrollX[NUM_BACKGROUNDS] = {0};
    float speeds[NUM_BACKGROUNDS] = {0.2f, 0.4f, 0.6f, 0.8f, 1.0f, 1.2f}; // vitesses différentes
    int bgWidths[NUM_BACKGROUNDS];

    char filename[32];
    for (int i = 0; i < NUM_BACKGROUNDS; i++) {
        snprintf(filename, sizeof(filename), "bg%d.png", i);
        backgrounds[i] = LoadTexture(filename, renderer);
        if (!backgrounds[i]) {
            printf("Erreur lors du chargement du fond %d\n", i);
            return 1;
        }
        SDL_QueryTexture(backgrounds[i], NULL, NULL, &bgWidths[i], NULL);
    }

    // Charger le sprite du chevalier
    SDL_Texture* knight = LoadTexture("knight.png", renderer);
    if (!knight) {
        printf("Erreur lors du chargement du sprite knight.png\n");
        return 1;
    }

    int running = 1;
    SDL_Event event;
    int frame = 0;
    Uint32 lastTime = SDL_GetTicks();
    Uint32 frameDuration = 150;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastTime >= frameDuration) {
            frame = (frame + 1) % 8;
            lastTime = currentTime;    
        }

        // Mettre à jour les positions de chaque background
        for (int i = 0; i < NUM_BACKGROUNDS; i++) {
            scrollX[i] -= speeds[i];
            if (scrollX[i] <= -bgWidths[i]) scrollX[i] = 0;
        }

        SDL_RenderClear(renderer);

        // Afficher chaque fond avec effet de boucle (scroll infini)
        for (int i = 0; i < NUM_BACKGROUNDS; i++) {
            SDL_Rect srcRect = {0, 0, bgWidths[i], WINDOW_HEIGHT};
            SDL_Rect dstRect1 = {(int)scrollX[i], 0, bgWidths[i], WINDOW_HEIGHT};
            SDL_Rect dstRect2 = {(int)scrollX[i] + bgWidths[i], 0, bgWidths[i], WINDOW_HEIGHT};

            SDL_RenderCopy(renderer, backgrounds[i], &srcRect, &dstRect1);
            SDL_RenderCopy(renderer, backgrounds[i], &srcRect, &dstRect2);
        }

        // Afficher le chevalier
        CreateKnight(knight, window, renderer, 100, 300, frame);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // Environ 60 FPS
    }

    // Libération mémoire
    for (int i = 0; i < NUM_BACKGROUNDS; i++) SDL_DestroyTexture(backgrounds[i]);
    SDL_DestroyTexture(knight);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();
    return 0;
}
