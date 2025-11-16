#include "endscreen.h"
#include <SDL2/SDL_image.h> 

#define END_SCREEN_SCALE_FACTOR 0.8f

/*
// Fonction privée pour l'effet de fondu (prise de votre exemple)
static void fadeIn(SDL_Renderer *renderer, SDL_Texture *texture) {
    // Obtenir les dimensions de l'écran
    int screen_w, screen_h;
    SDL_GetRendererOutputSize(renderer, &screen_w, &screen_h);

    // Obtenir les dimensions originales de la texture pour garder le bon ratio
    int tex_w, tex_h;
    SDL_QueryTexture(texture, NULL, NULL, &tex_w, &tex_h);

    // Calculer les nouvelles dimensions de l'image
    // La nouvelle largeur est un pourcentage de la largeur de l'écran
    int new_w = screen_w * END_SCREEN_SCALE_FACTOR;
    // La nouvelle hauteur est calculée proportionnellement pour ne pas déformer l'image
    // On fait le calcul en float pour la précision
    int new_h = new_w * ((float)tex_h / (float)tex_w);

    // Créer le rectangle de destination, CENTRÉ sur l'écran
    SDL_Rect dst = {
        .x = (screen_w - new_w) / 2,
        .y = (screen_h - new_h) / 2,
        .w = new_w,
        .h = new_h
    };

    // La boucle de fondu reste la même, mais utilisera le nouveau rectangle `dst`
    for (int alpha = 0; alpha <= 255; alpha += 5) {
        SDL_SetTextureAlphaMod(texture, alpha);
        
        // On dessine l'image par-dessus ce qui existe déjà (la scène de jeu)
        SDL_RenderCopy(renderer, texture, NULL, &dst);
        
        SDL_RenderPresent(renderer);
        SDL_Delay(20);
    }
}
*/

void show_end_screen(SDL_Renderer* renderer, const char* image_path) {
    SDL_Surface* surface = IMG_Load(image_path);
    if (!surface) {
        printf("Erreur chargement image d'écran de fin\n");
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        printf("Erreur création texture\n");
        return;
    }

    // Get texture dimensions
    int tex_w, tex_h;
    SDL_QueryTexture(texture, NULL, NULL, &tex_w, &tex_h);
    
    // Center on screen
    int screen_w, screen_h;
    SDL_GetRendererOutputSize(renderer, &screen_w, &screen_h);
    
    SDL_Rect dst_rect = {
        (screen_w - tex_w) / 2,
        (screen_h - tex_h) / 2,
        tex_w,
        tex_h
    };

    // Simple fade-in effect
    for (int alpha = 0; alpha <= 255; alpha += 5) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        SDL_SetTextureAlphaMod(texture, alpha);
        SDL_RenderCopy(renderer, texture, NULL, &dst_rect);
        SDL_RenderPresent(renderer);
        SDL_Delay(20);
    }

    // Display for 2 seconds
    SDL_Delay(2000);
    
    SDL_DestroyTexture(texture);
}