#ifndef EFFETSDL_H
#define EFFETSDL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


typedef struct {
    float re, im;
} Complex;

Complex zoom_inverse(Complex z_prime, Complex z_0, float a);
SDL_Surface* apply_zoom(SDL_Surface* src, float alpha, Complex z_0) ;
void complexe_vers_coordonnee_image(Complex z, int* x, int* y);
Complex coordonnee_image_vers_complexe(int x, int y);
Complex translation_inverse(Complex z_inv,float dx ,float  dy);
SDL_Surface* apply_trans(SDL_Surface* src, float x_t, float y_t) ;
SDL_Surface* apply_zoom_sur_zone(SDL_Surface* src, float alpha, SDL_Rect zone);
Complex rotation_img(Complex z, Complex z0 , float angle);

Complex rotation_inverse_d(Complex z, Complex z0, float Angle_max, int d0, int d1, int d_max);
SDL_Surface* apply_rotation(SDL_Surface* src, float angle, Complex z_0);
SDL_Surface* apply_rotation_d(SDL_Surface* src, float angle, int d0, int d1, int d_max, Complex z_0);
SDL_Surface* apply_rotation_sur_zone(SDL_Surface* src, float angle, SDL_Rect zone);

#endif