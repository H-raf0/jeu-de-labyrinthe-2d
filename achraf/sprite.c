#include <math.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
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


void ShowMovingLayer(SDL_Texture *my_texture, SDL_Rect window_dimensions, SDL_Renderer *renderer, int x) {

    SDL_Rect 
        source = {0},                         // Rectangle définissant la zone de la texture à récupérer
        destination = {0};                    // Rectangle définissant où la zone_source doit être déposée dans le renderer

    SDL_QueryTexture(my_texture, NULL, NULL,
             &source.w, &source.h);       // Récupération des dimensions de l'image


    destination = window_dimensions;              // On fixe les dimensions de l'affichage à  celles de la fenêtre
    destination.x = x;  
    destination.y = (window_dimensions.h - destination.h) / 2;  // La destination est au milieu de la hauteur de la fenêtre


    SDL_RenderCopy(renderer, my_texture,&source,&destination);                 // Création de l'élément à afficher
}

void play_with_texture_2(SDL_Texture* my_texture, SDL_Window* window,
                         SDL_Renderer* renderer) {
    SDL_Rect source =
                {0},  // Rectangle définissant la zone de la texture à récupérer
        window_dimensions = {0},  // Rectangle définissant la fenêtre, on
                                    // n'utilisera que largeur et hauteur
        destination = {0};  // Rectangle définissant où la zone_source doit être
                            // déposée dans le renderer

    SDL_GetWindowSize(
        window, &window_dimensions.w,
        &window_dimensions.h);  // Récupération des dimensions de la fenêtre
    SDL_QueryTexture(my_texture, NULL, NULL, &source.w,
                    &source.h);  // Récupération des dimensions de l'image

    float zoom = 1.5;                 // Facteur de zoom à appliquer
    destination.w = source.w * zoom;  // La destination est un zoom de la source
    destination.h = source.h * zoom;  // La destination est un zoom de la source
    destination.x =
        (window_dimensions.w - destination.w) /
        2;  // La destination est au milieu de la largeur de la fenêtre
    destination.y =
        (window_dimensions.h - destination.h) /
        2;  // La destination est au milieu de la hauteur de la fenêtre

    SDL_RenderCopy(renderer, my_texture,  // Préparation de l'affichage
                    &source, &destination);
    SDL_RenderPresent(renderer);
    SDL_Delay(1000);

    SDL_RenderClear(renderer);  // Effacer la fenêtre
}


int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Error : SDL initialisation - %s\n", SDL_GetError());              
        exit(EXIT_FAILURE);
    }

    SDL_Window* window = SDL_CreateWindow("Sprites",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH, SCREEN_HEIGHT,SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        SDL_Log("Error : Window creation - %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_Log("Error : Renderer creation - %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    
    // ici le code d'animation

    SDL_Rect window_dimensions = {0};

    SDL_GetWindowSize(window,              // Récupération des dimensions de la fenêtre
            &window_dimensions.w,
            &window_dimensions.h);

    SDL_Texture * bg= load_texture_from_image("./img/Background.png",window,renderer);

    int speed1 = 1;
    int x1=0,x2=window_dimensions.w;
    //play_with_texture_2(bg, window, renderer);
    while (1) {
        

        SDL_RenderClear(renderer);
        ShowMovingLayer(bg, window_dimensions, renderer, x1);
        ShowMovingLayer(bg, window_dimensions, renderer, x2);
        SDL_RenderPresent(renderer);
        
        x1 -= 1*speed1;
        x2 -= 1*speed1;
        if(x1 == -window_dimensions.w-1) x1 = window_dimensions.w;
        if(x2 == -window_dimensions.w-1) x2 = window_dimensions.w;
        SDL_Delay(1); 
    }
    
    /*SDL_RenderClear(renderer);           // Effacer l'image précédente avant de dessiner la nouvelle
    ShowMovingLayer(bg, window_dimensions, renderer);
    SDL_RenderPresent(renderer); 
    SDL_Delay(200); */


    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(bg);
    SDL_Quit();

    return 0;
}