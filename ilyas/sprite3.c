#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define COUCHES_NB 4


// Fonction pour gérer proprement la fin du programme
void end_sdl(char ok, const char* msg, SDL_Window* window, SDL_Renderer* renderer) {
    if (!ok) {
        SDL_Log("%s : %s\n", msg, SDL_GetError());
    }

    if (renderer != NULL) SDL_DestroyRenderer(renderer);
    if (window != NULL) SDL_DestroyWindow(window);

    SDL_Quit();
    if (!ok) exit(EXIT_FAILURE);
}

// Charge une texture à partir d'un fichier image
SDL_Texture* load_texture_from_image(char* file_image_name, SDL_Window* window, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(file_image_name);
    if (surface == NULL) end_sdl(0, "Erreur chargement image", window, renderer);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == NULL) end_sdl(0, "Erreur création texture", window, renderer);

    return texture;
}

// Affiche une couche de fond avec décalage horizontal
void ShowLayer(SDL_Texture* texture, SDL_Rect window_dim, SDL_Renderer* renderer, int x, int y) {
    SDL_Rect src = {0}, dst = {0};
    SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);

    dst = window_dim;
    dst.x = x;
    dst.y = y;

    SDL_RenderCopy(renderer, texture, &src, &dst);
}

// Affiche une vignette du sprite du dragon
void Create_eagle(SDL_Texture* texture, SDL_Window* window, SDL_Renderer* renderer, int posX, int posY, int frameIndex) {
    SDL_Rect src = {0}, dst = {0}, win_dim = {0};

    SDL_GetWindowSize(window, &win_dim.w, &win_dim.h);
    SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);

    int columns = 2, rows = 1;
    float zoom = 1.5f;
    int frameW = src.w / columns;
    int frameH = src.h / rows;

    src.x = frameIndex * frameW; // colonne (frameIndex+1)
    src.y = 0 * frameH;  // Ligne 1
    src.w = frameW;
    src.h = frameH;

    dst.w = frameW * zoom;
    dst.h = frameH * zoom;
    dst.x = posX;
    dst.y = posY;

    SDL_RenderCopy(renderer, texture, &src, &dst);
}

void InitialisationSDL(SDL_Window** window, SDL_Renderer** renderer, SDL_Rect* window_dimensions){
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Erreur initialisation SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    *window = SDL_CreateWindow("fenetre", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) end_sdl(0, "Erreur création fenêtre", NULL, NULL);

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!(*renderer)) end_sdl(0, "Erreur création renderer", *window, NULL);

    SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);  // fond noir

    // recuperer les dimension de l'ecran
    SDL_Rect win_dim = {0};
    SDL_GetWindowSize(*window, &win_dim.w, &win_dim.h);
    *window_dimensions = win_dim;
}

SDL_Texture** chargerCouche(SDL_Window* window, SDL_Renderer* renderer){
    SDL_Texture** couches = malloc(sizeof(SDL_Texture*)*COUCHES_NB);
    for (int i = 0; i < COUCHES_NB; i++) {
        char filename[100];
        sprintf(filename, "/home/local.isima.fr/ilnasrat/shared/projetzz1aiz/Nature Landscapes Free Pixel Art/nature_3/%d.png", i + 1);
        couches[i] = load_texture_from_image(filename, window, renderer);
    }
    return couches;
}

SDL_Texture* charger_eagl(SDL_Window* window, SDL_Renderer* renderer){
    return load_texture_from_image("/home/local.isima.fr/ilnasrat/Téléchargements/eagle.png", window, renderer);
}

void destroyLayers_and_eagle(SDL_Texture** layers, SDL_Texture** eagle){

    // Libération des ressources
    for (int i = 0; i < COUCHES_NB; i++) {
        SDL_DestroyTexture(layers[i]);
    }
    SDL_DestroyTexture(*eagle);
}

void destroyAndQuit(SDL_Window** window, SDL_Renderer** renderer){
    SDL_DestroyRenderer(*renderer);
    SDL_DestroyWindow(*window);

    IMG_Quit();
    SDL_Quit();

}
int main() {

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Rect window_dimensions = {0};
    InitialisationSDL(&window, &renderer, &window_dimensions);
    // Chargement des couches de fond
    SDL_Texture** layers = chargerCouche(window, renderer);  
    // Chargement du sprite du dragon
    SDL_Texture* eagle = charger_eagl(window, renderer);




    int eagle_frame = 0, frameCount = 0;
    int eagle_pos[2] = {window_dimensions.w / 4, window_dimensions.h * 2 / 8};
    int x1[COUCHES_NB], x2[COUCHES_NB];
    float speeds[COUCHES_NB] = {4, 3, 2, 1};  // vitesse de mouvement des couches De l'avant vers l’arriere
    
    // Initialisation des positions des couches
    for (int i = 0; i < COUCHES_NB; i++) {
        x1[i] = 0;
        x2[i] = window_dimensions.w;
    }

    //condition d'arret
    int running = 1;


    SDL_Event event;
    
    // Boucle principale
    while (running) {

        // Gestion des événements clavier
        const Uint8* state = SDL_GetKeyboardState(NULL);

        // Récupération des événements (fermeture, etc.)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = 0;
        }

        // Lecture des touches pressées en continu
        if (state[SDL_SCANCODE_UP])    eagle_pos[1] -= 10;
        if (state[SDL_SCANCODE_DOWN])  eagle_pos[1] += 10;
        if (state[SDL_SCANCODE_LEFT])  eagle_pos[0] -= 10;
        if (state[SDL_SCANCODE_RIGHT]) eagle_pos[0] += 10;

        SDL_RenderClear(renderer);

        // Affichage des couches
        for (int i = COUCHES_NB - 1; i >= 0; i--) {
            ShowLayer(layers[i], window_dimensions, renderer, x1[i], 0);
            ShowLayer(layers[i], window_dimensions, renderer, x2[i], 0);

            x1[i] -= speeds[i];
            x2[i] -= speeds[i];

            if (x1[i] < -window_dimensions.w) x1[i] += 2 * window_dimensions.w;
            if (x2[i] < -window_dimensions.w) x2[i] += 2 * window_dimensions.w;
        }

        // Affichage du dragon
        Create_eagle(eagle, window, renderer, eagle_pos[0], eagle_pos[1], eagle_frame % 2);

        // Changement de sprite toutes les 10 frames
        if (frameCount % 10 == 0) eagle_frame++;

        SDL_RenderPresent(renderer);
        SDL_Delay(20);
        frameCount++;
    }
    destroyLayers_and_eagle(layers, &eagle);
    destroyAndQuit(&window, &renderer); 
    free(layers);
    return 0;
}

