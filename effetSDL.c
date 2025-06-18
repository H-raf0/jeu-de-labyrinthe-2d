#include "effetSDL.h"

Complex zoom_inverse(Complex z_prime, Complex z_0, float a) {
    Complex z = {((z_prime.re - z_0.re) / a) + z_0.re, ((z_prime.im - z_0.im) / a) + z_0.im};
    return z;
}