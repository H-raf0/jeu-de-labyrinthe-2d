#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define MAX_COINS 10

void end_sdl(char ok, const char* msg, SDL_Window* window, SDL_Renderer* renderer) {
    char msg_formated[255];
    int l;

    if (!ok) {
        strncpy(msg_formated, msg, 250);
        l = strlen(msg_formated);
        strcpy(msg_formated + l, " : %s\n");
        SDL_Log(msg_formated, SDL_GetError());
    }

    if (renderer != NULL) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window != NULL) {
        SDL_DestroyWindow(window);
        window = NULL;
    }

    SDL_Quit();
    if (!ok) exit(EXIT_FAILURE);
}

SDL_Texture* load_texture_from_image(char* file_image_name, SDL_Window* window, SDL_Renderer* renderer) {
    SDL_Surface* my_image = IMG_Load(file_image_name);
    if (my_image == NULL) end_sdl(0, "Chargement de l'image impossible", window, renderer);

    SDL_Texture* my_texture = SDL_CreateTextureFromSurface(renderer, my_image);
    SDL_FreeSurface(my_image);
    if (my_texture == NULL) end_sdl(0, "Echec de la transformation de la surface en texture", window, renderer);

    return my_texture;
}

void ShowLayer(SDL_Texture* texture, SDL_Rect window_dim, SDL_Renderer* renderer, int x, int y) {
    SDL_Rect src = {0}, dst = {0};
    SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);

    dst = window_dim;
    dst.x = x;
    dst.y = y;

    SDL_RenderCopy(renderer, texture, &src, &dst);
}

void Createrenard(SDL_Texture* texture, SDL_Window* window, SDL_Renderer* renderer, int posX, int posY, int frameIndex) {
    SDL_Rect src = {0}, dst = {0}, win_dim = {0};

    SDL_GetWindowSize(window, &win_dim.w, &win_dim.h);
    SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);

    int columns = 3, rows = 4;
    float zoom = 4.0f;
    int frameW = src.w / columns;
    int frameH = src.h / rows;

    src.x = frameIndex * frameW;
    src.y = 1 * frameH;
    src.w = frameW;
    src.h = frameH;

    dst.w = frameW * zoom;
    dst.h = frameH * zoom;
    dst.x = posX;
    dst.y = posY;

    SDL_RenderCopy(renderer, texture, &src, &dst);
}
/*
int main() {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Erreur initialisation SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        SDL_Log("Erreur initialisation SDL_image : %s\n", IMG_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    SDL_Window* window = SDL_CreateWindow("Desert et renard", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) end_sdl(0, "Erreur création fenêtre", NULL, NULL);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) end_sdl(0, "Erreur création renderer", window, NULL);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect window_dimensions = {0};
    SDL_GetWindowSize(window, &window_dimensions.w, &window_dimensions.h);

    SDL_Texture* layers[5];
    for (int i = 0; i < 5; i++) {
        char filename[100];
        sprintf(filename, "/home/local.isima.fr/ilnasrat/shared/projetzz1aiz/ilyas/foret(1).png");
        layers[i] = load_texture_from_image(filename, window, renderer);
    }

    SDL_Texture* renard = load_texture_from_image("/home/local.isima.fr/ilnasrat/shared/projetzz1aiz/ilyas/fox-1.1/PNG/48x64/fox-NESW-bright.png", window, renderer);

    SDL_Texture* coin_texture = load_texture_from_image("/home/local.isima.fr/ilnasrat/shared/projetzz1aiz/ilyas/goldCoin1.png", window, renderer);

    SDL_Rect coins[MAX_COINS];
    bool collected[MAX_COINS] = {false};
    int score = 0;

    for (int i = 0; i < MAX_COINS; i++) {
        coins[i].w = 32;
        coins[i].h = 32;
        coins[i].x = 600 + i * 300;
        coins[i].y = 550 - (rand() % 100); // entre 450 et 550
    }

    int renardFrame = 0, frameCount = 0;
    int renardY = 635;
    float velocityY = 0;
    bool isJumping = false;
    float gravity = 1.0f;
    float jumpStrength = -30.0f;

    int x1[5], x2[5];
    float speeds[5] = {5, 4, 3, 2, 1};

    for (int i = 0; i < 5; i++) {
        x1[i] = 0;
        x2[i] = window_dimensions.w;
    }

    SDL_Event event;
    bool running = true;

    while (running && frameCount <= 1000) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && !isJumping) {
                isJumping = true;
                velocityY = jumpStrength;
            }
        }

        SDL_RenderClear(renderer);

        for (int i = 4; i >= 0; i--) {
            ShowLayer(layers[i], window_dimensions, renderer, x1[i], 0);
            ShowLayer(layers[i], window_dimensions, renderer, x2[i], 0);

            x1[i] -= speeds[i];
            x2[i] -= speeds[i];

            if (x1[i] < -window_dimensions.w) x1[i] += 2 * window_dimensions.w;
            if (x2[i] < -window_dimensions.w) x2[i] += 2 * window_dimensions.w;
        }

        if (isJumping) {
            renardY += (int)velocityY;
            velocityY += gravity;

            if (renardY >= 635) {
                renardY = 635;
                isJumping = false;
            }
        }

        // Affichage des pièces
        for (int i = 0; i < MAX_COINS; i++) {
            if (!collected[i]) {
                SDL_RenderCopy(renderer, coin_texture, NULL, &coins[i]);
            }
        }

        // Affichage du renard
        Createrenard(renard, window, renderer, 700, renardY, renardFrame % 3);
        SDL_Rect renard_box = {700, renardY, 48 * 4, 64 * 4};

        // Collision
        for (int i = 0; i < MAX_COINS; i++) {
            if (!collected[i] && SDL_HasIntersection(&renard_box, &coins[i])) {
                collected[i] = true;
                score++;
                printf("Pièce ramassée ! Score : %d\n", score);
            }
        }

        if (frameCount % 10 == 0) renardFrame++;

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
        frameCount++;
    }

    for (int i = 0; i < 5; i++) SDL_DestroyTexture(layers[i]);
    SDL_DestroyTexture(renard);
    SDL_DestroyTexture(coin_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
*/
int main() {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Erreur initialisation SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        SDL_Log("Erreur initialisation SDL_image : %s\n", IMG_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    SDL_Window* window = SDL_CreateWindow("Desert et renard", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) end_sdl(0, "Erreur création fenêtre", NULL, NULL);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) end_sdl(0, "Erreur création renderer", window, NULL);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect window_dimensions = {0};
    SDL_GetWindowSize(window, &window_dimensions.w, &window_dimensions.h);

    SDL_Texture* layers[5];
    for (int i = 0; i < 5; i++) {
        char filename[100];
        sprintf(filename, "/home/local.isima.fr/ilnasrat/shared/projetzz1aiz/ilyas/foret(1).png");
        layers[i] = load_texture_from_image(filename, window, renderer);
    }

    SDL_Texture* renard = load_texture_from_image("/home/local.isima.fr/ilnasrat/shared/projetzz1aiz/ilyas/fox-1.1/PNG/48x64/fox-NESW-bright.png", window, renderer);
    SDL_Texture* coin_texture = load_texture_from_image("/home/local.isima.fr/ilnasrat/shared/projetzz1aiz/ilyas/goldCoin1.png", window, renderer);

    SDL_Rect coins[MAX_COINS];
    bool collected[MAX_COINS] = {false};
    int score = 0;

    for (int i = 0; i < MAX_COINS; i++) {
        coins[i].w = 32;
        coins[i].h = 32;
        coins[i].x = 600 + i * 300;
        coins[i].y = 550 - (rand() % 100);
    }

    int renardFrame = 0, frameCount = 0;
    int renardX = 700;
    int renardY = 635;
    float velocityY = 0;
    bool isJumping = false;
    float gravity = 1.0f;
    float jumpStrength = -25.0f;
    int renardSpeed = 10;
    bool movingLeft = false, movingRight = false;

    int x1[5], x2[5];
    float speeds[5] = {5, 4, 3, 2, 1};

    for (int i = 0; i < 5; i++) {
        x1[i] = 0;
        x2[i] = window_dimensions.w;
    }

    SDL_Event event;
    bool running = true;

    while (running && frameCount <= 10000) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE && !isJumping) {
                    isJumping = true;
                    velocityY = jumpStrength;
                }
                else if (event.key.keysym.sym == SDLK_LEFT) movingLeft = true;
                else if (event.key.keysym.sym == SDLK_RIGHT) movingRight = true;
            }
            else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_LEFT) movingLeft = false;
                else if (event.key.keysym.sym == SDLK_RIGHT) movingRight = false;
            }
        }

        SDL_RenderClear(renderer);

        for (int i = 4; i >= 0; i--) {
            ShowLayer(layers[i], window_dimensions, renderer, x1[i], 0);
            ShowLayer(layers[i], window_dimensions, renderer, x2[i], 0);

            x1[i] -= speeds[i];
            x2[i] -= speeds[i];

            if (x1[i] < -window_dimensions.w) x1[i] += 2 * window_dimensions.w;
            if (x2[i] < -window_dimensions.w) x2[i] += 2 * window_dimensions.w;
        }

        if (movingLeft) renardX -= renardSpeed;
        if (movingRight) renardX += renardSpeed;

        if (isJumping) {
            renardY += (int)velocityY;
            velocityY += gravity;

            if (renardY >= 635) {
                renardY = 635;
                isJumping = false;
            }
        }

        // Affichage des pièces
        for (int i = 0; i < MAX_COINS; i++) {
            if (!collected[i]) {
                SDL_RenderCopy(renderer, coin_texture, NULL, &coins[i]);
            }
        }

        // Affichage du renard
        Createrenard(renard, window, renderer, renardX, renardY, renardFrame % 3);
        SDL_Rect renard_box = {renardX, renardY, 48 * 4, 64 * 4};

        // Collision avec pièces
        for (int i = 0; i < MAX_COINS; i++) {
            if (!collected[i] && SDL_HasIntersection(&renard_box, &coins[i])) {
                collected[i] = true;
                score++;
                printf("Pièce ramassée ! Score : %d\n", score);
            }
        }

        // Animation de marche
        if ((movingLeft || movingRight) && frameCount % 10 == 0) renardFrame++;

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
        frameCount++;
    }

    for (int i = 0; i < 5; i++) SDL_DestroyTexture(layers[i]);
    SDL_DestroyTexture(renard);
    SDL_DestroyTexture(coin_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
