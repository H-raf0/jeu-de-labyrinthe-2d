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
#define NUM_STARS 100

typedef struct {
    int x, y;
} Star;

void draw_circle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int angle = 0; angle < 360; angle++) {
        float rad = angle * M_PI / 180.0f;
        int x = cx + cos(rad) * radius;
        int y = cy + sin(rad) * radius;
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

void draw_sun(SDL_Renderer* renderer, int cx, int cy, float rotation_angle) {
    SDL_SetRenderDrawColor(renderer, 255, 223, 0, 255); // Jaune
    for (int r = 0; r < SUN_RADIUS; r++) {
        draw_circle(renderer, cx, cy, r);
    }

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

void draw_moon(SDL_Renderer* renderer, int cx, int cy) {
    // Lune grise
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    for (int r = 0; r < SUN_RADIUS; r++) {
        draw_circle(renderer, cx, cy, r);
    }

    // Ombre pour simuler un croissant
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int r = 0; r < SUN_RADIUS - 5; r++) {
        draw_circle(renderer, cx + 10, cy, r);
    }
}

void draw_stars(SDL_Renderer* renderer, Star stars[], int count) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Blanc
    for (int i = 0; i < count; i++) {
        SDL_RenderDrawPoint(renderer, stars[i].x, stars[i].y);
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    srand(time(NULL));

    SDL_Window* window = SDL_CreateWindow("Soleil et Lune", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int sun_x = -SUN_RADIUS;
    int sun_y = HEIGHT / 3;

    float ray_angle = 0;
    bool isNight = false;

    Star stars[NUM_STARS];
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x = rand() % WIDTH;
        stars[i].y = rand() % HEIGHT;
    }

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                isNight = !isNight;
            }
        }

        sun_x += SUN_SPEED;
        if (sun_x - SUN_RADIUS > WIDTH) {
            sun_x = -SUN_RADIUS;
        }

        ray_angle += 0.01f;

        // Fond
        if (!isNight) {
            SDL_SetRenderDrawColor(renderer, 135, 206, 250, 255); // Bleu ciel
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 30, 255); // Nuit sombre
        }
        SDL_RenderClear(renderer);

        if (!isNight) {
            draw_sun(renderer, sun_x, sun_y, ray_angle);
        } else {
            draw_stars(renderer, stars, NUM_STARS);
            draw_moon(renderer, sun_x, sun_y);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
