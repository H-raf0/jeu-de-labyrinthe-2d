#include "effetSDL.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <math.h>









Complex coordonnee_image_vers_complexe(int x, int y) {
    Complex z = {
        .re = x,
        .im = y
    };
    return z;
}

void complexe_vers_coordonnee_image(Complex z, int* x, int* y) {
    *x = (int) roundf(z.re);
    *y = (int) roundf(z.im);
}


//===================zooooooooooom=========================

Complex zoom_inverse(Complex z_prime, Complex z_0, float a) {
    Complex z = {((z_prime.re - z_0.re) / a) + z_0.re, ((z_prime.im - z_0.im) / a) + z_0.im};
    return z;
}


SDL_Surface* apply_zoom(SDL_Surface* src, float alpha, Complex z_0) {
    SDL_Surface* converted_src = NULL;
    SDL_Surface* working_src = src;

    // Conversion  en RGBA32
    if (src->format->format != SDL_PIXELFORMAT_RGBA32) {
        converted_src = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGBA32, 0);
        if (!converted_src) {
            printf("Error converting surface format: %s\n", SDL_GetError());
            return NULL;
        }
        working_src = converted_src;
    }

    // Création de dest au même format que working_src
    SDL_Surface* dest = SDL_CreateRGBSurfaceWithFormat(
        0,
        working_src->w,
        working_src->h,
        working_src->format->BitsPerPixel,
        working_src->format->format
    );
    if (!dest) {
        printf("Error creating destination surface: %s\n", SDL_GetError());
        if (converted_src) SDL_FreeSurface(converted_src);
        return NULL;
    }


    // Récupération des pointeurs et pitch
    Uint8* src_pixels  = (Uint8*)working_src->pixels;
    Uint8* dest_pixels = (Uint8*)dest->pixels;
    int   bpp          = working_src->format->BytesPerPixel;
    int   src_pitch    = working_src->pitch;
    int   dest_pitch   = dest->pitch;

    // Boucle de zoom inverse
    for (int j = 0; j < dest->h; j++) {
        for (int i = 0; i < dest->w; i++) {
            Complex z_dest = coordonnee_image_vers_complexe(i, j);
            Complex z_src  = zoom_inverse(z_dest, z_0, alpha);

            int x_src = (int)roundf(z_src.re);
            int y_src = (int)roundf(z_src.im);

            Uint8* dest_p = dest_pixels + j * dest_pitch + i * bpp;
            if (x_src >= 0 && x_src < working_src->w &&
                y_src >= 0 && y_src < working_src->h) {
                Uint8* src_p = src_pixels + y_src * src_pitch + x_src * bpp;
                memcpy(dest_p, src_p, bpp);
            } else {
                memset(dest_p, 0, bpp);
            }
        }
    }


    // Nettoyage conversion
    if (converted_src) SDL_FreeSurface(converted_src);

    return dest;
}


Complex zoom_inverse_d(Complex z_prime, Complex z0, float alpha_max, float d0, float d_max) {
    float mod_z = sqrt(pow(z_prime.re - z0.re, 2) + pow(z_prime.im - z0.im, 2));
    float scale = 1.0; // Par défaut, pas de zoom

    if (mod_z <= d0) {
        // Zone centrale : pas de zoom
        return z_prime;
    } else if (mod_z <= d_max) {
        if(alpha_max >=1){
            // Décroissance linéaire de alpha_max à 1.0
            scale = alpha_max - (alpha_max - 1.0) * (mod_z - d0) / (d_max - d0);
        }else{
            scale = alpha_max + (1.0 - alpha_max) * (mod_z - d0) / (d_max - d0);
        }
    }

    // Application du zoom
    Complex result;
    result.re = z0.re + (z_prime.re - z0.re) * scale;
    result.im = z0.im + (z_prime.im - z0.im) * scale;
    return result;
}

// ?
/*
Complex zoom_inverse_d_continu(Complex z_prime, Complex z0, float alpha_max, float d0, float d_max, float d1) {
    // Calcul de la distance entre z_prime et z0
    float dx = z_prime.re - z0.re;
    float dy = z_prime.im - z0.im;
    float dist = sqrtf(dx * dx + dy * dy);

    float facteur = 1.0f;

    // Constantes de pente pour la montée et la descente
    const float lambda_monte = 2.0f;
    const float lambda_descend = 2.0f;

    // Zone sans mise à l’échelle
    if (dist <= d0) {
        return z_prime;
    }

    // Zone de montée : d0 < dist <= d_max
    if (dist <= d_max) {
        float plage = (d_max - d0);
        if (plage <= 0.0f) {
            facteur = alpha_max;
        } else {
            float t = (dist - d0) / plage;
            float exp_moins = expf(-lambda_monte);
            float norm = 1.0f - exp_moins;
            if (norm == 0.0f) {
                facteur = 1.0f + (alpha_max - 1.0f) * t;
            } else {
                float numer = 1.0f - expf(-lambda_monte * t);
                facteur = 1.0f + (alpha_max - 1.0f) * (numer / norm);
            }
        }
    }
    // Zone de descente : d_max < dist <= d1
    else if (dist <= d1) {
        float plage = (d1 - d_max);
        if (plage <= 0.0f) {
            facteur = alpha_max;
        } else {
            float t = (dist - d_max) / plage;
            float exp_moins = expf(-lambda_descend);
            float norm = 1.0f - exp_moins;
            if (norm == 0.0f) {
                facteur = alpha_max + (1.0f - alpha_max) * t;
            } else {
                float numer = expf(-lambda_descend * t) - exp_moins;
                facteur = 1.0f + (alpha_max - 1.0f) * (numer / norm);
            }
        }
    } else {
        return z_prime;
    }

    // Application du facteur d’échelle
    Complex resultat;
    resultat.re = z0.re + dx * facteur;
    resultat.im = z0.im + dy * facteur;
    return resultat;
}
*/

SDL_Surface* apply_zoom_partiel(SDL_Surface* src, float alpha, Complex z0, float d0, float dmax) {
    SDL_Surface* converted_src = NULL;
    SDL_Surface* working_src = src;

    // Conversion  en RGBA32
    if (src->format->format != SDL_PIXELFORMAT_RGBA32) {
        converted_src = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGBA32, 0);
        if (!converted_src) {
            printf("Error converting surface format: %s\n", SDL_GetError());
            return NULL;
        }
        working_src = converted_src;
    }

    // Création de dest au même format que working_src
    SDL_Surface* dest = SDL_CreateRGBSurfaceWithFormat(
        0,
        working_src->w,
        working_src->h,
        working_src->format->BitsPerPixel,
        working_src->format->format
    );
    if (!dest) {
        printf("Error creating destination surface: %s\n", SDL_GetError());
        if (converted_src) SDL_FreeSurface(converted_src);
        return NULL;
    }


    // Récupération des pointeurs et pitch
    Uint8* src_pixels  = (Uint8*)working_src->pixels;
    Uint8* dest_pixels = (Uint8*)dest->pixels;
    int   bpp          = working_src->format->BytesPerPixel;
    int   src_pitch    = working_src->pitch;
    int   dest_pitch   = dest->pitch;

    // Boucle de zoom inverse
    for (int j = 0; j < dest->h; j++) {
        for (int i = 0; i < dest->w; i++) {
            Complex z_dest = coordonnee_image_vers_complexe(i, j);
            Complex z_src  = zoom_inverse_d(z_dest, z0, alpha, d0, dmax);

            int x_src = (int)roundf(z_src.re);
            int y_src = (int)roundf(z_src.im);

            Uint8* dest_p = dest_pixels + j * dest_pitch + i * bpp;
            if (x_src >= 0 && x_src < working_src->w &&
                y_src >= 0 && y_src < working_src->h) {
                Uint8* src_p = src_pixels + y_src * src_pitch + x_src * bpp;
                memcpy(dest_p, src_p, bpp);
            } else {
                memset(dest_p, 0, bpp);
            }
        }
    }


    // Nettoyage conversion
    if (converted_src) SDL_FreeSurface(converted_src);

    return dest;
}

//=========================translation=====================================

Complex translation_inverse(Complex z_inv,float dx ,float dy){
    Complex z;
    
    z.re=(z_inv.re) - dx;
    z.im=(z_inv.im) - dy;
    return z;
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


//===============================rotation==============================================

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

    // Get pixel access information
    const int src_bpp = working_src->format->BytesPerPixel;
    const int dest_bpp = dest->format->BytesPerPixel;
    Uint8* src_pixels = (Uint8*)working_src->pixels;
    Uint8* dest_pixels = (Uint8*)dest->pixels;

    // Process each pixel
    for (int y = 0; y < dest->h; y++) { 
        for (int x = 0; x < dest->w; x++) {
            Complex z_dest = coordonnee_image_vers_complexe(x, y);
            Complex z_src = rotation_img(z_dest, z_0, angle);
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


    // Free the temporary converted surface if we created one
    if (converted_src) {
        SDL_FreeSurface(converted_src);
    }

    return dest;
}


Complex rotation_inverse_d(Complex z_prime, Complex z0, float Angle_max, float d0, float d1, float d_max){
    float mod_z = pow(pow(z_prime.re - z0.re, 2) + pow(z_prime.im - z0.im, 2), 0.5);
    float angle = 0;
    if (mod_z <= d0){
        return z_prime;
    }else if((mod_z <= d_max) && (mod_z > d0)){
        angle = (Angle_max/(d_max - d0))*(mod_z - d0);
        return rotation_img(z_prime, z0, angle);
    }else if((mod_z < d1) && (mod_z > d_max)){
        angle = (Angle_max/(d_max - d1))*(mod_z - d_max) + Angle_max;
        return rotation_img(z_prime, z0, angle);
    }else{
        return z_prime;
    }
}

//
Complex rotation_inverse_d_continu(Complex z_prime, Complex z0, float theta0, float lambda, float T) {
    float dx = z_prime.re - z0.re;
    float dy = z_prime.im - z0.im;
    float mod_z = sqrt(dx * dx + dy * dy);

    if (lambda <= 0.0f || lambda >= 1.0f) {
        // Valeur de lambda invalide
        return z_prime;
    }

    // K = T / ln(lambda⁻¹) = T / ln(1/lambda)
    float K = T / log(1.0f / lambda);

    if (mod_z > T) {
        return z_prime;
    }

    // θ(z) = θ₀ × exp(-|z| / K)
    float angle = theta0 * exp(-mod_z / K);
    
    // ?
    if (fabs(angle) < 1e-2) angle = 0;
    
    return rotation_img(z_prime, z0, angle);
}


SDL_Surface* apply_rotation_d_continu(SDL_Surface* src, float angle, float lambda, float t, Complex z_0) {
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
            Complex z_src = rotation_inverse_d_continu(z_dest, z_0, angle, lambda, t);
            
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

SDL_Surface* apply_rotation_d(SDL_Surface* src, float angle, float d0, float d1, float d_max, Complex z_0) {
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

/*
SDL_Surface* apply_rotation_sur_zone(SDL_Surface* src, float angle_deg, SDL_Rect zone) {
    if (zone.x < 0 || zone.y < 0 || zone.w <= 0 || zone.h <= 0 || 
        zone.x + zone.w > src->w || zone.y + zone.h > src->h) {
        printf("Error: Invalid zone parameters\n");
        return NULL;
    }

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

    // Destination : copie complète de l'image source
    SDL_Surface* dest = SDL_ConvertSurface(working_src, working_src->format, 0);
    if (!dest) {
        printf("Error creating destination surface: %s\n", SDL_GetError());
        if (converted_src) SDL_FreeSurface(converted_src);
        return NULL;
    }


    Uint32* src_pixels = (Uint32*)working_src->pixels;
    Uint32* dest_pixels = (Uint32*)dest->pixels;
    int src_pitch = working_src->pitch / 4;
    int dest_pitch = dest->pitch / 4;

    float angle_rad = angle_deg * M_PI / 180.0f;

    Complex z_0 = {
        .re = zone.x + zone.w / 2.0f,
        .im = zone.y + zone.h / 2.0f
    };

    for (int y = 0; y < zone.h; y++) {
        for (int x = 0; x < zone.w; x++) {
            Complex zp = {
                .re = zone.x + x,
                .im = zone.y + y
            };

            Complex z = rotation_img(zp, z_0, angle_rad);

            int x_src = (int)floor(z.re);
            int y_src = (int)floor(z.im);

            if (x_src >= 0 && x_src < working_src->w &&
                y_src >= 0 && y_src < working_src->h) {
                dest_pixels[(zone.y + y) * dest_pitch + (zone.x + x)] =
                    src_pixels[y_src * src_pitch + x_src];
            } else {
                dest_pixels[(zone.y + y) * dest_pitch + (zone.x + x)] = 0; // transparent
            }
        }
    }


    if (converted_src) SDL_FreeSurface(converted_src);

    return dest;
}

*/











