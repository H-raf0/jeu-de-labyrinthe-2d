#include <SDL2/SDL.h>
#include <stdio.h>
#define N 3

void Init_SDL(){
    /* Initialisation de la SDL  + gestion de l'échec possible */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Error : SDL initialisation - %s\n", 
                SDL_GetError());                // l'initialisation de la SDL a échoué 
    exit(EXIT_FAILURE);
    }
}

SDL_Window* CreateWindow(int posX, int posY, int length, int height, SDL_Renderer** renderer) {
    SDL_Window *window = NULL;                     // Future fenêtre de gauche

    /* Création de la fenêtre */
    window = SDL_CreateWindow(
    "Fenêtre",                    // codage en utf8, donc accents possibles
    posX, posY,                                  // coin haut gauche en haut gauche de l'écran
    length, height,                              // largeur, hauteur
    SDL_WINDOW_RESIZABLE);                 // 

    if (window == NULL) {
        SDL_Log("Error : SDL window 1 creation - %s\n", 
                    SDL_GetError());                 // échec de la création de la fenêtre
        SDL_Quit();                              // On referme la SDL       
        exit(EXIT_FAILURE);
    }
    
    *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (*renderer == NULL) {
        SDL_Log("Error : SDL renderer creation - %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);
    SDL_RenderClear(*renderer);
    SDL_RenderPresent(*renderer);

    return window;
    
}

int* GetResolution() {
    // Get the display count
    int displayCount = SDL_GetNumVideoDisplays();
    if (displayCount < 1) {
        printf("No displays available.\n");
        SDL_Quit();
        return NULL; // Returning NULL when no displays are available
    }

    // Dynamically allocate memory for resolution
    int* resolution = (int*)malloc(2 * sizeof(int));
    if (resolution == NULL) {
        printf("Memory allocation failed.\n");
        SDL_Quit();
        return NULL; // Handle memory allocation failure
    }

    // Loop through each display and get its current mode
    for (int i = 0; i < displayCount; ++i) {
        SDL_DisplayMode mode;
        if (SDL_GetCurrentDisplayMode(i, &mode) != 0) {
            printf("SDL_GetCurrentDisplayMode failed for display %d: %s\n", i, SDL_GetError());
        } else {
            // Store width and height in the allocated array
            resolution[0] = mode.w;
            resolution[1] = mode.h;
            return resolution;
        }
    }

    // If we get here, it means we couldn't retrieve the resolution
    resolution[0] = 0;
    resolution[1] = 0;
    return resolution;
}

void CreateSerpent(SDL_Window **windowsArray, SDL_Renderer **renderersArray, int taille, int pointDepX, int pointDepY){

    for(int i=0; i<N; i++){

        windowsArray[i] = CreateWindow(pointDepX-i*90, pointDepY, taille, taille, &renderersArray[i]);
    }
}



void MoveSerpent(SDL_Window **windowsArray, int taille) {
    int x, y, x_new=0, y_new=0, pasX = 30, pasY = 30;
    int tabPos[N][5];
    for (int i = 0; i < N; i++) {
        SDL_GetWindowPosition(windowsArray[i], &x, &y);
        tabPos[i][0] = x;
        tabPos[i][1] = y - 37;
        tabPos[i][2] = pasX;
        tabPos[i][3] = pasY;
        tabPos[i][4] = taille;
    }
    for (int j = 0; j < 100; j++) {
        for (int i = 0; i < N; i++) {
            
            printf("Position %d: %d, %d\n",i, tabPos[i][0], tabPos[i][1]);

            x_new = tabPos[i][0] + tabPos[i][2];
            y_new = tabPos[i][1] - 37 + tabPos[i][3];

            // Si la fenêtre dépasse la largeur de l'écran, changer la direction
            if (x_new >= 1830 || x_new <= 0) {  // Si x arrive à sa limite dans l ecran
                tabPos[i][2] *= -1;
                tabPos[i][4] -= 20;
                SDL_SetWindowSize(windowsArray[i], tabPos[i][4], tabPos[i][4]);
            }
            // Si la fenêtre dépasse la hauteur de l'écran, changer la direction
            if (y_new >= 940 || y_new <= 0) {  // Si y arrive à sa limite dans l ecran
                tabPos[i][3] *= -1;
                tabPos[i][4] -= 20;
                SDL_SetWindowSize(windowsArray[i], tabPos[i][4], tabPos[i][4]);
            }
            // Déplacer la fenêtre
            tabPos[i][0] += tabPos[i][2];
            tabPos[i][1] += tabPos[i][3];
            SDL_SetWindowPosition(windowsArray[i], tabPos[i][0], tabPos[i][1]);
            
        }
        SDL_Delay(80); // Attendre un peu avant de déplacer à nouveau
    }
}


void DestroyWindow(SDL_Window *window, SDL_Renderer *renderer){

    /* on referme tout ce qu'on a ouvert en ordre inverse de la création */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


int main(){
    int nbWindows = 3;
    int taille = 100;
    Init_SDL();
    SDL_Window **windowsArray = (SDL_Window **) malloc(sizeof(SDL_Window *) * nbWindows);
    SDL_Renderer **renderersArray = (SDL_Renderer **)malloc(sizeof(SDL_Renderer *) * nbWindows);
    
    int* res = GetResolution();
    printf("%d %d\n", res[0], res[1]);  
    CreateSerpent(windowsArray, renderersArray, taille, res[0]/2, res[1]/2);
    MoveSerpent(windowsArray, taille);
    SDL_Delay(1000);

    for(int i=nbWindows-1; i>=0; i--){
        
        DestroyWindow(windowsArray[i], renderersArray[i]);
    }

    free(res);
    free(windowsArray);
    free(renderersArray);
    

    SDL_Quit();                                // quitter la SDL
    return 0;

}


//free resolution