#include <math.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define layersNb 5

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
//191  161
void CreateDragon(SDL_Texture* my_texture, SDL_Window* window, SDL_Renderer* renderer, int posX, int posY, int textNb) {
    //fun to pepare stuff
    SDL_Rect 
        source = {0},                    // Rectangle définissant la zone totale de la planche
        window_dimensions = {0},         // Rectangle définissant la fenêtre, on n'utilisera que largeur et hauteur
        destination = {0},               // Rectangle définissant où la zone_source doit être déposée dans le renderer
        state = {0};                     // Rectangle de la vignette en cours dans la planche 

    SDL_GetWindowSize(window,              // Récupération des dimensions de la fenêtre
            &window_dimensions.w,
            &window_dimensions.h);
    SDL_QueryTexture(my_texture,           // Récupération des dimensions de l'image
            NULL, NULL,
            &source.w, &source.h);


    /* Mais pourquoi prendre la totalité de l'image, on peut n'en afficher qu'un morceau, et changer de morceau :-) */

    int nb_images = 3;                     // nombre d'image par ligne
    int nb_ligne = 4;
    float zoom = 1.5;                        // zoom, car ces images sont un peu petites
    int offset_x = source.w / nb_images,   // La largeur d'une vignette de l'image, marche car la planche est bien réglée
        offset_y = source.h / nb_ligne;           // La hauteur d'une vignette de l'image, marche car la planche est bien réglée


    state.x = 0 ;                          // La première vignette est en début de ligne
    state.y = 1 * offset_y;                // On s'intéresse à la 2ème ligne
    state.w = offset_x;                    // Largeur de la vignette
    state.h = offset_y;                    // Hauteur de la vignette

    destination.w = offset_x * zoom;       // Largeur du sprite à l'écran
    destination.h = offset_y * zoom;       // Hauteur du sprite à l'écran

    
    destination.y =  posY;                      // La course se fait en bas d'écran (en vertical)

    destination.x = posX;
    state.x = offset_x * textNb;
    SDL_RenderCopy(renderer, my_texture, // Préparation de l'affichage
                &state,
                &destination);  
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

    //load layer
    SDL_Texture* layers[layersNb];

    for (int i = 0; i < layersNb; i++) {
        char filename[100];
        sprintf(filename, "./img/winter/%d.png", i + 1);
        layers[i] = load_texture_from_image(filename, window, renderer);
    }
    SDL_Texture* dragonTxt = load_texture_from_image("./img/dragon/drg.png", window, renderer);

    //int x1=0,x2=window_dimensions.w;
    int dragonTextNb = 0, j=1;
    int x1[layersNb];  // Position de chaque couche
    int x2[layersNb];  // Position de chaque couche
    float speed[layersNb] = {5, 4, 3, 2, 1};  // Vitesse de fond -> avant-plan

    for (int i = 0; i < layersNb; i++) {
        x1[i] = 0;
        x2[i] = window_dimensions.w;
    }
    
    while (j<=700) {
        

        
        SDL_RenderClear(renderer);
        for (int i = layersNb-1; i >= 0; i--) {
            ShowMovingLayer(layers[i], window_dimensions, renderer, (int) x1[i]);
            ShowMovingLayer(layers[i], window_dimensions, renderer, (int) x2[i]);
            x1[i] -= speed[i];
            x2[i] -= speed[i];
            if(x1[i] < -window_dimensions.w) x1[i] += 2 * window_dimensions.w;
            if(x2[i] < -window_dimensions.w) x2[i] += 2 * window_dimensions.w;
        }
        CreateDragon(dragonTxt, window, renderer, window_dimensions.w/4, window_dimensions.h*2/8,dragonTextNb%3);
        if (j%10==0) dragonTextNb++; //vitesse de changement des sprites du dragon
        SDL_RenderPresent(renderer);
        j++;
        SDL_Delay(16); 
    }
    
    



    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    for (int i = 0; i < layersNb; i++) {
        SDL_DestroyTexture(layers[i]);
    }
    SDL_DestroyTexture(dragonTxt);
    IMG_Quit();
    SDL_Quit();

    return 0;
}