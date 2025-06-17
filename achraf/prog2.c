#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

// Fonction de terminaison propre
void end_sdl(char ok, const char* msg, SDL_Window* window, SDL_Renderer* renderer) {
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

// Dessine un cercle avec des points
void drawCercle(SDL_Renderer* renderer, int x, int y, int diametre) {
    for (float angle = 0; angle < 2 * M_PI; angle += M_PI / 4000) {
        SDL_RenderDrawPoint(renderer,
                            x + diametre * cos(angle),
                            y + diametre * sin(angle));
    }
}

// Dessine plusieurs cercles autour d’un point
void drawLinkedCercles(SDL_Renderer* renderer, int x, int y, int diametre, int nbCercle) {
    static int angle = 0;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Couleur blanche
    for (int i = 0; i < nbCercle; i++) {
        float offsetAngle = angle + (2 * M_PI / nbCercle) * i;
        drawCercle(renderer,
                   x + diametre * 2 * cos(offsetAngle),
                   y + diametre * 2 * sin(offsetAngle),
                   diametre);
    }

    angle += 6;  // Vitesse de rotation
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_DisplayMode screen;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) end_sdl(0, "ERROR SDL INIT", window, renderer);
    SDL_GetCurrentDisplayMode(0, &screen);
    printf("Résolution écran\n\tw : %d\n\th : %d\n", screen.w, screen.h);

    window = SDL_CreateWindow("Cercles interactifs",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              screen.w, screen.h,
                              SDL_WINDOW_SHOWN);
    if (window == NULL) end_sdl(0, "ERROR WINDOW CREATION", window, renderer);

    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) end_sdl(0, "ERROR RENDERER CREATION", window, renderer);

    // Animation principale
    double t, x, y;
    int N = 300,i =0;
    int running = 1;
    int nbCercles = 16;

    SDL_Event event;

    while (running) {
        // Gestion des événements clavier
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    running = 0;
                else if (event.key.keysym.sym == SDLK_SPACE)
                    nbCercles = (nbCercles == 16) ? 32 : 16;  // Toggle entre 16 et 32 cercles
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        t = 2 * M_PI * i / N;
        x = screen.w / 2 + 500 * pow(cos(t), 3);
        y = screen.h / 2 + 500 * pow(sin(t), 3);
		i++;
		
        drawLinkedCercles(renderer, x, y, 40, nbCercles);

        SDL_RenderPresent(renderer);
        SDL_Delay(10);
		
    }

    end_sdl(1, "Normal ending", window, renderer);
    return EXIT_SUCCESS;
}
