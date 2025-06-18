#ifndef EFFETSDL_H
#define EFFETSDL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct {
    float re, im;
} Complex;

Complex zoom_inverse(Complex z_prime, Complex z_0, float a);
SDL_Surface* apply_zoom(SDL_Surface* src, float alpha, Complex z_0);
void complexe_vers_coordonnee_image(Complex z, int* x, int* y);
Complex coordonnee_image_vers_complexe(int x, int y);
Complex translation_inverse(Complex z_inv,int dx ,int  dy);
SDL_Surface* apply_trans(SDL_Surface* src, int x_t, int y_t);
SDL_Surface* apply_zoom_sur_zone(SDL_Surface* src, float alpha, SDL_Rect zone);
#endif