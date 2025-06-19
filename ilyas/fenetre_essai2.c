#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define HAUTEUR_MIN 200
#define HAUTEUR_MAX 600
#define LARGEUR_MIN 300
#define LARGEUR_MAX 800
#define PAS 5

void Init_SDL() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Erreur init SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

SDL_Window* Create_Window(int posX, int posY, int length, int height, SDL_Renderer** renderer) {
    SDL_Window* window = SDL_CreateWindow("Fenêtre",
                                          posX, posY,
                                          length, height,
                                          SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Erreur création fenêtre : %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {
        SDL_Log("Erreur création renderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    return window;
}

void random_resize(SDL_Window* window, int* w, int* h) {
    *w = LARGEUR_MIN + rand() % (LARGEUR_MAX - LARGEUR_MIN + 1);
    *h = HAUTEUR_MIN + rand() % (HAUTEUR_MAX - HAUTEUR_MIN + 1);
    SDL_SetWindowSize(window, *w, *h);
}

void random_color(SDL_Renderer* renderer) {
    int r = rand() % 256;
    int g = rand() % 256;
    int b = rand() % 256;
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void move_window_bouncing(SDL_Window* window, SDL_Renderer* renderer) {
    SDL_DisplayMode screen;
    SDL_GetCurrentDisplayMode(0, &screen);
    int screenW = screen.w;
    int screenH = screen.h;

    int x = 100, y = 100;
    int dx = PAS, dy = PAS;
    int w = 400, h = 300;

    SDL_SetWindowSize(window, w, h);
    SDL_SetWindowPosition(window, x, y);

    random_color(renderer);

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;
        }

        x += dx;
        y += dy;

        bool bounced = false;

        if (x <= 0 || x + w >= screenW) {
            dx = -dx;
            bounced = true;
        }

        if (y <= 0 || y + h >= screenH) {
            dy = -dy;
            bounced = true;
        }

        if (bounced) {
            random_resize(window, &w, &h);

            // Réajuster position si dépasse bord
            if (x + w > screenW) x = screenW - w;
            if (y + h > screenH) y = screenH - h;

            random_color(renderer);
        }

        SDL_SetWindowPosition(window, x, y);
        SDL_Delay(16);  // ~60 FPS
    }
}

int main() {
    Init_SDL();
    srand((unsigned int)time(NULL));

    SDL_Renderer* renderer = NULL;
    SDL_Window* window = Create_Window(100, 100, 400, 300, &renderer);

    move_window_bouncing(window, renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
