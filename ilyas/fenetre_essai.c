#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include <math.h>
#include <time.h>
#define hauteur_min 200
#define hauteur_max 1000
#define largeur_min 300
#define longueur_max 1000 
#define PAS 5 
#define L 1980 
#define H 1080 





void Init_SDL()
{
    /* Initialisation de la SDL  + gestion de l'echec possible */
    if(SDL_Init(SDL_INIT_VIDEO)!=0)
    {
        // l'initialisation de la SDL a echoue
        SDL_Log("Error : SDL initialisation - %s\n",SDL_GetError());
        exit(EXIT_FAILURE);
    }
}
SDL_Window * Create_Window (int posX, int posY, int length, int height, SDL_Renderer ** renderer)
{
    SDL_Window * window;
    window = SDL_CreateWindow("Fenetre",posX, posY, length, height,SDL_WINDOW_RESIZABLE);
    if (window == NULL)
    {
        SDL_Log("Error : SDL window creation - %s\n", 
                    SDL_GetError());                 // echec de la creation de la fenetre
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

    return window;
}

void random_resize(SDL_Window* window, int * x,int * y)
{
    *x = largeur_min + rand()%(longueur_max - largeur_min + 1);
    *y = hauteur_min + rand()%(hauteur_max - hauteur_min + 1);
} 
// Change la couleur de fond aleatoirement
void random_color(SDL_Renderer* renderer)
{
    int r = rand() % 256;
    int g = rand() % 256;
    int b = rand() % 256;
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}
void move_window_bouncing(SDL_Window* window, SDL_Renderer* renderer) 
{
    int x = 100, y = 100;
    int dx = PAS, dy = PAS;
    int w = 400, h = 300;

    SDL_SetWindowSize(window, w, h);

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) 
        {
            if (e.type == SDL_QUIT) quit = true;
        }

        x += dx;
        y += dy;

        bool bounced = false;

        if (x <= 0 || x + w >= L) {
            dx = -dx;
            bounced = true;
        }
        if (y <= 0 || y + h >= H) {
            dy = -dy;
            bounced = true;
        }

        if (bounced) {
            random_resize(window, &w, &h);
            random_color(renderer);
        }

        SDL_SetWindowPosition(window, x, y);
        SDL_Delay(16);
    }
}



void free_window(SDL_Window * window, SDL_Renderer *renderer)
{
    //s'il y a rien "pas de renderer"
    if (renderer == NULL)
    {
        SDL_Log("Error : SDL window creation - %s\n", 
                    SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);

    }
    //s'il y a rien "pas de window"
    if (window == NULL)
    {
        SDL_Log("Error : SDL window creation - %s\n", 
                    SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    //on fait un " destroy" de windows
    SDL_DestroyWindow(window);
    SDL_Quit();
}


int main()
{
    Init_SDL();
    srand((unsigned int)time(NULL)); // Initialisation du generateur aleatoire
    SDL_Renderer* renderer = NULL;
    SDL_Window* window = Create_Window(100, 100, 200, 200, &renderer);
    if (!window || !renderer) {
        SDL_Quit();
        return 1;
    }

    random_color(renderer); // couleur initiale

    move_window_bouncing(window, renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}