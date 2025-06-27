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
    SDL_Surface* original_surface = IMG_Load(image_path);
    if (!original_surface) {
        printf("Erreur chargement image d'écran de fin\n");
        return;
    }

    //Définir le centre de rotation comme le centre de l'image.
    Complex center_of_rotation = { 
        .re = original_surface->w / 2.0f,
        .im = original_surface->h / 2.0f
    };

    float angle_rad = 0.0f;

    // Boucle d'animation principale 
    while (angle_rad <= 6.34 ) {
        
        // Appliquer la rotation à la surface ORIGINALE pour créer une nouvelle surface tournée.
        SDL_Surface* rotated_surface = apply_rotation(original_surface, angle_rad, center_of_rotation);
        if (!rotated_surface) {
            printf("L'application de la rotation a échoué.\n");
            return;
        }

        // Convertir la surface tournée en texture pour pouvoir la dessiner avec le renderer.
        SDL_Texture* frame_texture = SDL_CreateTextureFromSurface(renderer, rotated_surface);
        if(!frame_texture){
            printf("La création de la texture a échoué.\n");
            SDL_FreeSurface(rotated_surface); // Nettoyage avant de continuer.
            continue;
        }

        // Dessiner la texture à l'écran.
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fond noir.
        SDL_RenderClear(renderer);
        
        // Centre l'image à l'écran.
        int screen_w, screen_h;
        SDL_GetRendererOutputSize(renderer, &screen_w, &screen_h);
        SDL_Rect dst_rect = {
            (screen_w - rotated_surface->w) / 2,
            (screen_h - rotated_surface->h) / 2,
            rotated_surface->w,
            rotated_surface->h
        };
        SDL_RenderCopy(renderer, frame_texture, NULL, &dst_rect);
        SDL_RenderPresent(renderer);

        // Nettoyage DANS LA BOUCLE (critique pour éviter les fuites de mémoire).
        // On libère la surface et la texture créées spécifiquement pour cette frame.
        SDL_FreeSurface(rotated_surface);
        SDL_DestroyTexture(frame_texture);

        // Mettre à jour l'angle pour la prochaine frame.
        angle_rad += 0.08f; // Vitesse de rotation 
        SDL_Delay(16);
    }


    // On libère la surface originale qui a servi de modèle pendant toute l'animation.
    SDL_FreeSurface(original_surface);

    // Petite pause supplémentaire à la fin
    SDL_Delay(1000);
}