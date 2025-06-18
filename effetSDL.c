#include "effetSDL.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>

Complex zoom_inverse(Complex z_prime, Complex z_0, float a) {
    Complex z = {((z_prime.re - z_0.re) / a) + z_0.re, ((z_prime.im - z_0.im) / a) + z_0.im};
    return z;
}

Complex translation_inverse(Complex z_inv,int dx ,int dy){
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
SDL_Surface* apply_trans(SDL_Surface* src, int x_t, int y_t) {
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
            z_src = translation_inverse(z_dest, x_t, y_t);
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

