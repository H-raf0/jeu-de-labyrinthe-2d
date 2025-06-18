#ifndef EFFETSDL_H
#define EFFETSDL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct {
    float re, im;
} Complex;

Complex zoom_inverse(Complex z_prime, Complex z_0, float a);

#endif