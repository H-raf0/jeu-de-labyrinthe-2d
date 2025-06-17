#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL_image.h>
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080



void end_sdl(char ok,                                               // fin normale : ok = 0 ; anormale ok = 1
    char const* msg,                                       // message à afficher
    SDL_Window* window,                                    // fenêtre à fermer
    SDL_Renderer* renderer) {                              // renderer à fermer
    char msg_formated[255];                                            
    int l;                                                     
                                            
    if (!ok) {                                                        // Affichage de ce qui ne va pas
        strncpy(msg_formated, msg, 250);                                         
        l = strlen(msg_formated);                                            
        strcpy(msg_formated + l, " : %s\n");                                     
                                            
        SDL_Log(msg_formated, SDL_GetError());                                   
    }                                                          
                                            
    if (renderer != NULL) {                                           // Destruction si nécessaire du renderer
        SDL_DestroyRenderer(renderer);                                  // Attention : on suppose que les NULL sont maintenus !!
        renderer = NULL;
    }
    if (window != NULL)   {                                           // Destruction si nécessaire de la fenêtre
        SDL_DestroyWindow(window);                                      // Attention : on suppose que les NULL sont maintenus !!
        window= NULL;
    }
                                            
    SDL_Quit();                                                    
                                            
    if (!ok) {                                       // On quitte si cela ne va pas            
        exit(EXIT_FAILURE);                                     
    }                                                          
}  



SDL_Texture* load_texture_from_image(char  *  file_image_name, SDL_Window *window, SDL_Renderer *renderer ){
    SDL_Surface *my_image = NULL;           // Variable de passage
    SDL_Texture* my_texture = NULL;         // La texture

    my_image = IMG_Load(file_image_name);   // Chargement de l'image dans la surface
                                            // image=SDL_LoadBMP(file_image_name); fonction standard de la SDL, 
                                            // uniquement possible si l'image est au format bmp */
    if (my_image == NULL) end_sdl(0, "Chargement de l'image impossible", window, renderer);
   
    my_texture = SDL_CreateTextureFromSurface(renderer, my_image); // Chargement de l'image de la surface vers la texture
    SDL_FreeSurface(my_image);                                     // la SDL_Surface ne sert que comme élément transitoire 
    if (my_texture == NULL) end_sdl(0, "Echec de la transformation de la surface en texture", window, renderer);

    return my_texture;
}




void sync_image_fenetre(SDL_Texture *my_texture, SDL_Window *window,
               SDL_Renderer *renderer) {
    SDL_Rect 
        source = {0},                         // Rectangle définissant la zone de la texture à récupérer
        window_dimensions = {0},              // Rectangle définissant la fenêtre, on n'utilisera que largeur et hauteur
        destination = {0};                    // Rectangle définissant où la zone_source doit être déposée dans le renderer

    SDL_GetWindowSize(
    window, &window_dimensions.w,
    &window_dimensions.h);                    // Récupération des dimensions de la fenêtre
    SDL_QueryTexture(my_texture, NULL, NULL,
             &source.w, &source.h);       // Récupération des dimensions de l'image

    destination = window_dimensions;              // On fixe les dimensions de l'affichage à  celles de la fenêtre

    /* On veut afficher la texture de façon à ce que l'image occupe la totalité de la fenêtre */

    SDL_RenderCopy(renderer, my_texture,
           &source,
           &destination);                 // Création de l'élément à afficher
    SDL_RenderPresent(renderer);                  // Affichage
    SDL_Delay(2000);                              // Pause en ms

    SDL_RenderClear(renderer);                    // Effacer la fenêtre
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

    src.x = frameIndex * frameW; // colonne (frameIndex+1)
    src.y = 1 * frameH;  // Ligne 2
    src.w = frameW;
    src.h = frameH;

    dst.w = frameW * zoom;
    dst.h = frameH * zoom;
    dst.x = posX;
    dst.y = posY;

    SDL_RenderCopy(renderer, texture, &src, &dst);
}





int main()
{
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Erreur initialisation SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    

    SDL_Window* window = SDL_CreateWindow("Desert et renard", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) end_sdl(0, "Erreur création fenêtre", NULL, NULL);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) end_sdl(0, "Erreur création renderer", window, NULL);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // fond noir

    SDL_Rect window_dimensions = {0};
    SDL_GetWindowSize(window, &window_dimensions.w, &window_dimensions.h);

    // Chargement des couches de fond
    SDL_Texture* layers[5];
    for (int i = 0; i < 5; i++) {
        char filename[100];
        sprintf(filename, "/home/local.isima.fr/ilnasrat/Téléchargements/foret(1).png", i + 1);
        layers[i] = load_texture_from_image(filename, window, renderer);
    }

    // Chargement du sprite du renard
    SDL_Texture* renard = load_texture_from_image("/home/local.isima.fr/ilnasrat/Téléchargements/fox-1.1/PNG/48x64/fox-NESW-bright.png", window, renderer);

    int renardFrame = 0, frameCount = 0;
    int x1[5], x2[5];
    float speeds[5] = {5, 4, 3, 2, 1};  // vitesse de mouvement des couches De l'avant vers l’arriere

    // Initialisation des positions des couches
    for (int i = 0; i < 5; i++) {
        x1[i] = 0;
        x2[i] = window_dimensions.w;
    }

    // Boucle principale
    while (frameCount <= 700) {
        SDL_RenderClear(renderer);

        // Affichage des couches de fond (parallaxe)
        for (int i = 4; i >= 0; i--) {
            ShowLayer(layers[i], window_dimensions, renderer, x1[i], 0);
            ShowLayer(layers[i], window_dimensions, renderer, x2[i], 0);

            x1[i] -= speeds[i];
            x2[i] -= speeds[i];

            if (x1[i] < -window_dimensions.w) x1[i] += 2 * window_dimensions.w;
            if (x2[i] < -window_dimensions.w) x2[i] += 2 * window_dimensions.w;
        }

        // Affichage du renard
        Createrenard(renard, window, renderer, 700, 635, renardFrame % 3);

        // Changement de sprite toutes les 10 frames
        if (frameCount % 10 == 0) renardFrame++;

        SDL_RenderPresent(renderer);
        SDL_Delay(16);  // Environ 60 FPS
        frameCount++;
    }

    // Libération des ressources
    for (int i = 0; i < 5; i++) {
        SDL_DestroyTexture(layers[i]);
    }
    SDL_DestroyTexture(renard);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();

    return 0;
}