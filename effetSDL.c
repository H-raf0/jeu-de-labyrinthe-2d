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


// rotation_img prend déjà un angle en radians
Complex rotation_img(Complex z, Complex z0 , float theta){
    Complex resultat;
    float x = z.re;
    float y = z.im;
    float x0 = z0.re;
    float y0 = z0.im;

    resultat.re = cos(theta)*(x - x0) - sin(theta)*(y - y0) + x0;
    resultat.im = sin(theta)*(x - x0) + cos(theta)*(y - y0) + y0;
    return resultat;
}


Complex rotation_inverse_d(Complex z, Complex z0, float Angle_max, int d0, int d1, int d_max){
    float x = z.re;
    float y = z.im;
    float x0 = z0.re;
    float y0 = z0.im;

    float mod_z = sqrt((x - x0)*(x - x0) + (y - y0)*(y - y0));
    float theta_max = M_PI * Angle_max / 180;

    if (mod_z <= d0){   // pas de rotation
        return z;
    } else if (mod_z <= d_max){  // rotation croissante
        float angle = - theta_max / (d_max - d0) * (mod_z - d0);
        return rotation_img(z, z0, angle);
    } else if (mod_z <= d1){ // rotation decroissante
        float angle = - (theta_max / (d_max - d1) * (mod_z - d_max) + theta_max);
        return rotation_img(z, z0, angle);
    } else {  // pas de rotation
        return z;
    }
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



//zoom partiel
SDL_Surface* apply_zoom_sur_zone(SDL_Surface* src, float alpha, SDL_Rect zone) {
    // Vérification des paramètres de la zone
    if (zone.x < 0 || zone.y < 0 || zone.w <= 0 || zone.h <= 0 || 
        zone.x + zone.w > src->w || zone.y + zone.h > src->h) {
        printf("Error: Invalid zoom zone parameters\n");
        return NULL;
    }

    // Conversion de la surface source en format RGBA32 pour un traitement uniforme
    SDL_Surface* converted_src = NULL;
    SDL_Surface* working_src = src;
    
    if (src->format->format != SDL_PIXELFORMAT_RGBA32) {
        converted_src = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGBA32, 0);
        if (!converted_src) {
            printf("Error converting surface format: %s\n", SDL_GetError());
            return NULL;
        }
        working_src = converted_src;
    }

    // Création de la surface de destination
    SDL_Surface* dest = SDL_CreateRGBSurfaceWithFormat(0, zone.w, zone.h, 32, SDL_PIXELFORMAT_RGBA32);
    if (!dest) {
        printf("Error creating destination surface: %s\n", SDL_GetError());
        if (converted_src) SDL_FreeSurface(converted_src);
        return NULL;
    }

    // Verrouillage des surfaces si nécessaire
    if (SDL_MUSTLOCK(working_src)) SDL_LockSurface(working_src);
    if (SDL_MUSTLOCK(dest)) SDL_LockSurface(dest);

    // Accès aux pixels en 32-bit
    Uint32* src_pixels = (Uint32*)working_src->pixels;
    Uint32* dest_pixels = (Uint32*)dest->pixels;
    int src_pitch = working_src->pitch / 4;   // pitch en termes de pixels 32-bit
    int dest_pitch = dest->pitch / 4;

    // Centre du zoom
    Complex z_0 = {
        .re = zone.x + zone.w / 2.0f,
        .im = zone.y + zone.h / 2.0f
    };

    // Traitement pixel par pixel
    for (int y = 0; y < zone.h; y++) {
        for (int x = 0; x < zone.w; x++) {
            // Coordonnées dans l'image originale (sans zoom)
            Complex zp = {
                .re = (float)(zone.x + x),
                .im = (float)(zone.y + y)
            };
            
            // Application du zoom inverse
            Complex z = zoom_inverse(zp, z_0, alpha);
            
            // Conversion en coordonnées entières
            int x_src = (int)floor(z.re);
            int y_src = (int)floor(z.im);
            
            // Copie du pixel si dans les limites
            if (x_src >= 0 && x_src < working_src->w && y_src >= 0 && y_src < working_src->h) {
                dest_pixels[y * dest_pitch + x] = src_pixels[y_src * src_pitch + x_src];
            } else {
                // Noir transparent en dehors des limites
                dest_pixels[y * dest_pitch + x] = 0;
            }
        }
    }

    // Déverrouillage des surfaces
    if (SDL_MUSTLOCK(dest)) SDL_UnlockSurface(dest);
    if (SDL_MUSTLOCK(working_src)) SDL_UnlockSurface(working_src);
    
    // Nettoyage
    if (converted_src) {
        SDL_FreeSurface(converted_src);
    }

    return dest;
}

//translation avec x_t,y_t
SDL_Surface* apply_zoom(SDL_Surface* src, float alpha, Complex z_0) {
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
            Complex z_src = zoom_inverse(z_dest, z_0, alpha);
            
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



SDL_Surface* apply_rotation(SDL_Surface* src, float angle, Complex z_0) {
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
            Complex z_src = rotation_img(z_dest, z_0, -angle*M_PI/180);
            
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



SDL_Surface* apply_rotation_d(SDL_Surface* src, float angle, int d0, int d1, int d_max, Complex z_0) {
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
            Complex z_src = rotation_inverse_d(z_dest, z_0, angle, d0, d1, d_max);
            
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


SDL_Surface* apply_rotation_sur_zone(SDL_Surface* src, float angle, SDL_Rect zone) {
    // Vérification des paramètres de la zone
    if (zone.x < 0 || zone.y < 0 || zone.w <= 0 || zone.h <= 0 || 
        zone.x + zone.w > src->w || zone.y + zone.h > src->h) {
        printf("Error: Invalid zoom zone parameters\n");
        return NULL;
    }

    // Conversion de la surface source en format RGBA32 pour un traitement uniforme
    SDL_Surface* converted_src = NULL;
    SDL_Surface* working_src = src;
    
    if (src->format->format != SDL_PIXELFORMAT_RGBA32) {
        converted_src = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGBA32, 0);
        if (!converted_src) {
            printf("Error converting surface format: %s\n", SDL_GetError());
            return NULL;
        }
        working_src = converted_src;
    }

    // Création de la surface de destination
    SDL_Surface* dest = SDL_CreateRGBSurfaceWithFormat(0, zone.w, zone.h, 32, SDL_PIXELFORMAT_RGBA32);
    if (!dest) {
        printf("Error creating destination surface: %s\n", SDL_GetError());
        if (converted_src) SDL_FreeSurface(converted_src);
        return NULL;
    }

    // Verrouillage des surfaces si nécessaire
    if (SDL_MUSTLOCK(working_src)) SDL_LockSurface(working_src);
    if (SDL_MUSTLOCK(dest)) SDL_LockSurface(dest);

    // Accès aux pixels en 32-bit
    Uint32* src_pixels = (Uint32*)working_src->pixels;
    Uint32* dest_pixels = (Uint32*)dest->pixels;
    int src_pitch = working_src->pitch / 4;   // pitch en termes de pixels 32-bit
    int dest_pitch = dest->pitch / 4;

    // Centre du zoom
    Complex z_0 = {
        .re = zone.x + zone.w / 2.0f,
        .im = zone.y + zone.h / 2.0f
    };

    // Traitement pixel par pixel
    for (int y = 0; y < zone.h; y++) {
        for (int x = 0; x < zone.w; x++) {
            // Coordonnées dans l'image originale (sans zoom)
            Complex zp = {
                .re = (float)(zone.x + x),
                .im = (float)(zone.y + y)
            };
            
            // Application du zoom inverse
            Complex z = rotation_img(zp, z_0, angle*M_PI/180);
            
            // Conversion en coordonnées entières
            int x_src = (int)floor(z.re);
            int y_src = (int)floor(z.im);
            
            // Copie du pixel si dans les limites
            if (x_src >= 0 && x_src < working_src->w && y_src >= 0 && y_src < working_src->h) {
                dest_pixels[y * dest_pitch + x] = src_pixels[y_src * src_pitch + x_src];
            } else {
                // Noir transparent en dehors des limites
                dest_pixels[y * dest_pitch + x] = 0;
            }
        }
    }

    // Déverrouillage des surfaces
    if (SDL_MUSTLOCK(dest)) SDL_UnlockSurface(dest);
    if (SDL_MUSTLOCK(working_src)) SDL_UnlockSurface(working_src);
    
    // Nettoyage
    if (converted_src) {
        SDL_FreeSurface(converted_src);
    }

    return dest;
}












