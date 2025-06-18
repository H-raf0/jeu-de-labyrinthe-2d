#include "effetSDL.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>

Complex zoom_inverse(Complex z_prime, Complex z_0, float a) {
    Complex z = {((z_prime.re - z_0.re) / a) + z_0.re, ((z_prime.im - z_0.im) / a) + z_0.im};
    return z;
}

Complex translation_inverse(Complex z_inv,float dx ,float dy){
    Complex z;
    
    z.re=(z_inv.re) - dx;
    z.im=(z_inv.im) - dy;
    return z;
}


Complex coordonnee_image_vers_complexe(int x, int y) {
    Complex z = {
        .re = x,
        .im = y
    };
    return z;
}

void complexe_vers_coordonnee_image(Complex z, int* x, int* y) {
    *x = (int) floor(z.re);
    *y = (int) floor(z.im);
}

//zoom totale
SDL_Surface* apply_zoom(SDL_Surface* src, float alpha, Complex z_0) {
    if (SDL_MUSTLOCK(src)) SDL_LockSurface(src); //verrouiller l'access à src

    int x_src, y_src;
    Complex z_dest, z_src;
    
    // création de la surface de destination
    SDL_Surface* dest = SDL_CreateRGBSurfaceWithFormat(0, src->w, src->h, 32, src->format->format);
    if (!dest) {
        printf("Erreur création surface temporaire : %s\n", SDL_GetError());
        if (SDL_MUSTLOCK(src)) SDL_UnlockSurface(src);
        return NULL;
    }

    // Accès aux pixels avec gestion correcte du format
    int src_bpp = src->format->BytesPerPixel;
    int dest_bpp = dest->format->BytesPerPixel;

    Uint8* src_pixels = (Uint8*)src->pixels; // ensemble des pixels de src
    Uint8* dest_pixels = (Uint8*)dest->pixels; // ensemble des pixels de dest

    //loop sur les cordonnées de dest
    for (int y = 0; y < dest->h; y++) { 
        for (int x = 0; x < dest->w; x++) {
            z_dest = coordonnee_image_vers_complexe(x, y);
            z_src = zoom_inverse(z_dest, z_0, alpha);
            complexe_vers_coordonnee_image(z_src, &x_src, &y_src);

            // Calcul des positions mémoire
            Uint8* dest_p = dest_pixels + y * dest->pitch + x * dest_bpp;
            
            if (x_src >= 0 && x_src < src->w && y_src >= 0 && y_src < src->h) {
                Uint8* src_p = src_pixels + y_src * src->pitch + x_src * src_bpp;
                memcpy(dest_p, src_p, src_bpp);
            } else {
                // Noir opaque
                memset(dest_p, 0, dest_bpp);
            }
        }
    }

    
    if (SDL_MUSTLOCK(src)) SDL_UnlockSurface(src);

    return dest;
}

//zoom partiel
SDL_Surface* apply_zoom_sur_zone(SDL_Surface* src, float alpha, SDL_Rect zone) {
    if (SDL_MUSTLOCK(src)) SDL_LockSurface(src); //verrouiller l'access à src

    int x_src, y_src;
    Complex z_dest, z_src, z_0={zone.x + zone.w / 2.0f, zone.y + zone.h / 2.0f};
    
    // création de la surface de destination
    SDL_Surface* dest = SDL_CreateRGBSurfaceWithFormat(0, zone.w, zone.h, 32, src->format->format);
    if (!dest) {
        printf("Erreur création surface temporaire : %s\n", SDL_GetError());
        if (SDL_MUSTLOCK(src)) SDL_UnlockSurface(src);
        return NULL;
    }

    // Accès aux pixels avec gestion correcte du format
    int src_bpp = src->format->BytesPerPixel;
    int dest_bpp = dest->format->BytesPerPixel;

    Uint8* src_pixels = (Uint8*)src->pixels; // ensemble des pixels de src
    Uint8* dest_pixels = (Uint8*)dest->pixels; // ensemble des pixels de dest

    //loop sur les cordonnées de dest
    for (int y = 0; y < zone.h; y++) { 
        for (int x = 0; x < zone.w; x++) {
            z_dest = coordonnee_image_vers_complexe((zone.x + zone.w - 1)+x, (zone.y + zone.h - 1)+y);
            z_src = zoom_inverse(z_dest, z_0, alpha);
            complexe_vers_coordonnee_image(z_src, &x_src, &y_src);

            // Calcul des positions mémoire
            Uint8* dest_p = dest_pixels + y * dest->pitch + x * dest_bpp;
            
            if (x_src >= 0 && x_src < src->w && y_src >= 0 && y_src < src->h) {
                Uint8* src_p = src_pixels + y_src * src->pitch + x_src * src_bpp;
                memcpy(dest_p, src_p, src_bpp);
            } else {
                // Noir opaque
                memset(dest_p, 0, dest_bpp);
            }
        }
    }

    
    if (SDL_MUSTLOCK(src)) SDL_UnlockSurface(src);

    return dest;
}



//translation avec x_t,y_t
SDL_Surface* apply_trans(SDL_Surface* src, float x_t, float y_t) {
    // Create a temporary converted surface if needed
    SDL_Surface* converted_src = NULL;
    SDL_Surface* working_src = src;
    
    // Convert to RGBA32 if not already in that format
    if (src->format->format != SDL_PIXELFORMAT_RGBA32) {
        converted_src = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGBA32, 0);
        if (!converted_src) {
            printf("Error converting surface format: %s\n", SDL_GetError());
            return NULL;
        }
        working_src = converted_src;
    }

    // Create destination surface (same format as working source)
    SDL_Surface* dest = SDL_CreateRGBSurfaceWithFormat(0, working_src->w, working_src->h, 
                                                      working_src->format->BitsPerPixel, 
                                                      working_src->format->format);
    if (!dest) {
        printf("Error creating destination surface: %s\n", SDL_GetError());
        if (converted_src) SDL_FreeSurface(converted_src);
        return NULL;
    }

    // Lock surfaces if needed
    if (SDL_MUSTLOCK(working_src)) SDL_LockSurface(working_src);
    if (SDL_MUSTLOCK(dest)) SDL_LockSurface(dest);

    // Get pixel access information
    const int src_bpp = working_src->format->BytesPerPixel;
    const int dest_bpp = dest->format->BytesPerPixel;
    Uint8* src_pixels = (Uint8*)working_src->pixels;
    Uint8* dest_pixels = (Uint8*)dest->pixels;

    // Process each pixel
    for (int y = 0; y < dest->h; y++) { 
        for (int x = 0; x < dest->w; x++) {
            Complex z_dest = coordonnee_image_vers_complexe(x, y);
            Complex z_src = translation_inverse(z_dest, x_t, y_t);
            
            int x_src = 0, y_src = 0;
            complexe_vers_coordonnee_image(z_src, &x_src, &y_src);

            // Calculate memory positions
            Uint8* dest_p = dest_pixels + y * dest->pitch + x * dest_bpp;
            
            if (x_src >= 0 && x_src < working_src->w && y_src >= 0 && y_src < working_src->h) {
                Uint8* src_p = src_pixels + y_src * working_src->pitch + x_src * src_bpp;
                memcpy(dest_p, src_p, src_bpp);
            } else {
                // Set to transparent black (0) for out-of-bounds
                memset(dest_p, 0, dest_bpp);
            }
        }
    }

    // Unlock surfaces
    if (SDL_MUSTLOCK(dest)) SDL_UnlockSurface(dest);
    if (SDL_MUSTLOCK(working_src)) SDL_UnlockSurface(working_src);

    // Free the temporary converted surface if we created one
    if (converted_src) {
        SDL_FreeSurface(converted_src);
    }

    return dest;
}

