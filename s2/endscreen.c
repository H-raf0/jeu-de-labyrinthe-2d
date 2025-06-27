#include "endscreen.h"
#include <SDL2/SDL_image.h> // On utilise SDL_image pour être cohérent avec le reste du projet

#define END_SCREEN_SCALE_FACTOR 0.8f


// Fonction privée pour l'effet de fondu (prise de votre exemple)
static void fadeIn(SDL_Renderer *renderer, SDL_Texture *texture) {
    // 1. Obtenir les dimensions de l'écran
    int screen_w, screen_h;
    SDL_GetRendererOutputSize(renderer, &screen_w, &screen_h);

    // 2. Obtenir les dimensions originales de la texture pour garder le bon ratio
    int tex_w, tex_h;
    SDL_QueryTexture(texture, NULL, NULL, &tex_w, &tex_h);

    // 3. Calculer les nouvelles dimensions de l'image
    // La nouvelle largeur est un pourcentage de la largeur de l'écran
    int new_w = screen_w * END_SCREEN_SCALE_FACTOR;
    // La nouvelle hauteur est calculée proportionnellement pour ne pas déformer l'image
    // On fait le calcul en float pour la précision
    int new_h = new_w * ((float)tex_h / (float)tex_w);

    // 4. Créer le rectangle de destination, CENTRÉ sur l'écran
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

// Fonction publique pour afficher l'écran
void show_end_screen(SDL_Renderer* renderer, const char* image_path) {
    // On utilise IMG_LoadTexture pour charger des PNG, JPG, BMP, etc.
    SDL_Texture *end_texture = IMG_LoadTexture(renderer, image_path);
    if (!end_texture) {
        printf("Erreur chargement image d'écran de fin %s : %s\n", image_path, IMG_GetError());
        return;
    }

    // Activer le blending pour la transparence
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // Lancer l'effet
    fadeIn(renderer, end_texture);
    
    // Attendre un peu
    SDL_Delay(3000); // 3 secondes

    // Nettoyage
    SDL_DestroyTexture(end_texture);
}