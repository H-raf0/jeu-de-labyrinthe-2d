#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include <math.h>
#include <time.h>
#define WIDTH 800
#define HEIGHT 600
#define SUN_RADIUS 40
#define SUN_SPEED 2
#define NUM_RAYS 12
#define RAY_LENGTH 60

void draw_circle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int angle = 0; angle < 360; angle++)
    {
        float rad = angle * M_PI / 180.0f;
        int x = cx + cos(rad) * radius;
        int y = cy + sin(rad) * radius;
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

void draw_sun(SDL_Renderer* renderer, int cx, int cy, float rotation_angle) {
    // Cercle central du soleil
    SDL_SetRenderDrawColor(renderer, 255, 223, 0, 255); // Jaune
    for (int r = 0; r < SUN_RADIUS; r++) 
    {
        draw_circle(renderer, cx, cy, r);
    }

    // Rayons
    SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
    for (int i = 0; i < NUM_RAYS; i++) {
        float angle = i * (2 * M_PI / NUM_RAYS) + rotation_angle;
        int x1 = cx + cos(angle) * SUN_RADIUS;
        int y1 = cy + sin(angle) * SUN_RADIUS;
        int x2 = cx + cos(angle) * (SUN_RADIUS + RAY_LENGTH);
        int y2 = cy + sin(angle) * (SUN_RADIUS + RAY_LENGTH);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Soleil qui bouge", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int sun_x = -SUN_RADIUS;
    int sun_y = HEIGHT / 3;

    float ray_angle = 0;

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        // Gestion des événements
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;
        }

        // Mise à jour de la position du soleil
        sun_x += SUN_SPEED;
        if (sun_x - SUN_RADIUS > WIDTH) {
            sun_x = -SUN_RADIUS; // Recommence à gauche
        }

        ray_angle += 0.01f;

        // Fond du ciel
        SDL_SetRenderDrawColor(renderer, 135, 206, 250, 255); // Bleu ciel
        SDL_RenderClear(renderer);

        // Dessin du soleil
        draw_sun(renderer, sun_x, sun_y, ray_angle);

        // Affichage
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}