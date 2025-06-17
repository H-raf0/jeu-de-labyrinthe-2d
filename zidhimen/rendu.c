#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

void end_sdl(char ok, char const* msg, SDL_Window* window, SDL_Renderer* renderer) {
    char msg_formated[255];
    int l;
    if (!ok) {
        strncpy(msg_formated, msg, 250);
        l = strlen(msg_formated);
        strcpy(msg_formated + l, " : %s\n");
        SDL_Log(msg_formated, SDL_GetError());
    }
    if (renderer != NULL) SDL_DestroyRenderer(renderer);
    if (window != NULL) SDL_DestroyWindow(window);
    SDL_Quit();
    if (!ok) exit(EXIT_FAILURE);
}

void draw_circle(SDL_Renderer *renderer, float cx, float cy, float r) {
    for (float angle = 0; angle < 2 * M_PI; angle += 0.01f) {
        float x = cx + r * cos(angle);
        float y = cy + r * sin(angle);
        SDL_RenderDrawPoint(renderer, (int)x, (int)y);
    }
}

void draw_flower(SDL_Renderer *renderer, float cx, float cy, float petal_radius, int petal_count, float rotation_angle) {
    float angle_step = 2 * M_PI / petal_count;
    for (int i = 0; i < petal_count; i++) {
        float angle = i * angle_step + rotation_angle;  // Ajouter la rotation
        float px = cx + (petal_radius * 1.5f) * cos(angle);
        float py = cy + (petal_radius * 1.5f) * sin(angle);
        SDL_SetRenderDrawColor(renderer, 255, 105, 180, 255); // Rose vif
        draw_circle(renderer, px, py, petal_radius);
    }

    // Cœur de la fleur.
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Jaune
    draw_circle(renderer, cx, cy, petal_radius * 0.8f);
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        end_sdl(0, "Erreur initialisation SDL", window, renderer);

    window = SDL_CreateWindow("Fleur animée",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              800, 600,
                              SDL_WINDOW_SHOWN);
    if (!window)
        end_sdl(0, "Erreur création fenêtre", window, renderer);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        end_sdl(0, "Erreur création renderer", window, renderer);

    float cx = 400, cy = 300;  // Position de départ
    float vx = 2.0f, vy = 1.5f; // Vitesse
    float petal_radius = 30;
    int petal_count = 8;
    float rotation_angle = 0.0f;

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        // Effacer l'écran
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // fond noir
        SDL_RenderClear(renderer);

        // Mise à jour position
        cx += vx;
        cy += vy;

        // Rebonds sur les bords
        if (cx < 50 || cx > 750) vx = -vx;
        if (cy < 50 || cy > 550) vy = -vy;

        // Mise à jour angle de rotation
        rotation_angle += 0.05f;
        if (rotation_angle > 2 * M_PI) rotation_angle -= 2 * M_PI;

        // Dessiner la fleur
        draw_flower(renderer, cx, cy, petal_radius, petal_count, rotation_angle);

        // Afficher
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    end_sdl(1, "Fermeture normale", window, renderer);
    return EXIT_SUCCESS;
}
