#include <SDL2/SDL.h>
#include <stdio.h>

#define LARGEUR_GRILLE 10
#define N (LARGEUR_GRILLE * LARGEUR_GRILLE)
#define CELL_SIZE 50

void afficher_labyrinthe_sdl(SDL_Renderer* renderer, int carte_connue[N][N], int position_actuelle, int origine, int destination) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < N; i++) {
        int row = i / LARGEUR_GRILLE;
        int col = i % LARGEUR_GRILLE;

        int x = col * CELL_SIZE;
        int y = row * CELL_SIZE;

        // Couleur de fond
        if (i == origine) {
            SDL_SetRenderDrawColor(renderer, 144, 238, 144, 255);  // Vert clair
        } else if (i == destination) {
            SDL_SetRenderDrawColor(renderer, 135, 206, 250, 255);  // Bleu ciel
        } else {
            SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);  // Gris clair
        }

        SDL_Rect cell = { x, y, CELL_SIZE, CELL_SIZE };
        SDL_RenderFillRect(renderer, &cell);

        // Murs
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // noir

        // HAUT
        if (row == 0 || carte_connue[i][i - LARGEUR_GRILLE] == 0) {
            SDL_RenderDrawLine(renderer, x, y, x + CELL_SIZE, y);
        }
        // BAS
        if (row == LARGEUR_GRILLE - 1 || carte_connue[i][i + LARGEUR_GRILLE] == 0) {
            SDL_RenderDrawLine(renderer, x, y + CELL_SIZE, x + CELL_SIZE, y + CELL_SIZE);
        }
        // GAUCHE
        if (col == 0 || carte_connue[i][i - 1] == 0) {
            SDL_RenderDrawLine(renderer, x, y, x, y + CELL_SIZE);
        }
        // DROITE
        if (col == LARGEUR_GRILLE - 1 || carte_connue[i][i + 1] == 0) {
            SDL_RenderDrawLine(renderer, x + CELL_SIZE, y, x + CELL_SIZE, y + CELL_SIZE);
        }
    }

    // Agent (rouge)
    int agent_x = (position_actuelle % LARGEUR_GRILLE) * CELL_SIZE + CELL_SIZE / 4;
    int agent_y = (position_actuelle / LARGEUR_GRILLE) * CELL_SIZE + CELL_SIZE / 4;
    SDL_Rect agent = { agent_x, agent_y, CELL_SIZE / 2, CELL_SIZE / 2 };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &agent);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Labyrinthe avec murs", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          LARGEUR_GRILLE * CELL_SIZE, LARGEUR_GRILLE * CELL_SIZE, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int carte_connue[N][N] = {0};

    // Connexions de base (grille sans murs)
    for (int i = 0; i < N; i++) {
        int row = i / LARGEUR_GRILLE;
        int col = i % LARGEUR_GRILLE;

        if (row > 0) carte_connue[i][i - LARGEUR_GRILLE] = 1;
        if (row < LARGEUR_GRILLE - 1) carte_connue[i][i + LARGEUR_GRILLE] = 1;
        if (col > 0) carte_connue[i][i - 1] = 1;
        if (col < LARGEUR_GRILLE - 1) carte_connue[i][i + 1] = 1;
    }

    // Ajout de murs
    carte_connue[22][32] = carte_connue[32][22] = 0;
    carte_connue[32][42] = carte_connue[42][32] = 0;
    carte_connue[42][43] = carte_connue[43][42] = 0;

    int position_actuelle = 0;
    int origine = 0;
    int destination = 99;

    int quit = 0;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = 1;
        }

        afficher_labyrinthe_sdl(renderer, carte_connue, position_actuelle, origine, destination);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
