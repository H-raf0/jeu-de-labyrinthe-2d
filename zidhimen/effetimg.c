#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    float r; // partie réelle (x)
    float i; // partie imaginaire (y)
} Complex;

// Fonction de translation inverse : z' → z = z' - (dx, dy)
Complex translation_inverse(Complex z_inv, int dx, int dy) {
    Complex z;
    z.r = z_inv.r - dx;
    z.i = z_inv.i - dy;
    return z;
}

// Génération d'une image PPM avec ou sans transformation
void generate_image(const char* filename, int width, int height, int apply_transform) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // En-tête du format PPM P6
    fprintf(f, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Complex z_prime = {x, y};
            Complex z_result = z_prime;

            // Appliquer la transformation si demandé
            if (apply_transform) {
                z_result = translation_inverse(z_prime, 30, 5); // translation inverse de (30, 30)
            }

            int xi = (int)z_result.r;
            int yi = (int)z_result.i;

            unsigned char r = 0, g = 0, b = 0;

            // Vérifie que les coordonnées sont valides
            if (xi >= 0 && xi < width && yi >= 0 && yi < height) {
                // Motif de damier
                if ((xi / 50 + yi / 50) % 2 == 0) {
                    r = 255; // rouge
                } else {
                    g = 255; // vert
                }
            }

            fputc(r, f);
            fputc(g, f);
            fputc(b, f);
        }
    }

    fclose(f);
}

int main() {
    int width = 512, height = 512;

    generate_image("original.ppm", width, height, 0);      // Image sans transformation
    generate_image("translated.ppm", width, height, 1);    // Image avec translation inverse

    printf("Images générées (original.ppm et translated.ppm).\n");
    return 0;
}
