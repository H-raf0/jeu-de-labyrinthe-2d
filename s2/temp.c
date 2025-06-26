#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h> 

// Pointeur global pour notre effet sonore.
// On utilise un Mix_Chunk car c'est un son court, chargé en mémoire.
// Pour une longue musique de fond, on utiliserait Mix_Music.
Mix_Chunk *sonEffet = NULL;

/**
 * @brief Initialise SDL_mixer et charge le fichier son.
 * 
 * @param chemin_fichier_son Le chemin vers le fichier .mp3 ou .wav.
 * @return int 1 en cas de succès, 0 en cas d'échec.
 */
int init_audio(const char* chemin_fichier_son) {
    // Initialiser SDL_mixer
    // 44100 : fréquence en Hz (standard)
    // MIX_DEFAULT_FORMAT : format audio par défaut
    // 2 : nombre de canaux (stéréo)
    // 2048 : taille du buffer (une bonne valeur par défaut)
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Erreur d'initialisation de SDL_mixer: %s\n", Mix_GetError());
        return 0; // Échec
    }

    // Charger le fichier son dans notre Mix_Chunk
    // Mix_LoadWAV peut charger des WAV, MP3, OGG, etc.
    sonEffet = Mix_LoadWAV(chemin_fichier_son);
    if (sonEffet == NULL) {
        printf("Erreur de chargement du fichier son '%s': %s\n", chemin_fichier_son, Mix_GetError());
        Mix_CloseAudio();
        return 0; // Échec
    }

    printf("Audio initialisé et son chargé avec succès.\n");
    return 1; // Succès
}

/**
 * @brief Joue le son qui a été chargé.
 */
void jouer_son() {
    if (sonEffet != NULL) {
        // Joue le son sur le premier canal disponible (-1)
        // Le 0 signifie qu'on ne le joue pas en boucle (on le joue 1 fois).
        Mix_PlayChannel(-1, sonEffet, 0);
    } else {
        printf("Impossible de jouer le son : il n'a pas été chargé.\n");
    }
}

/**
 * @brief Libère la mémoire du son et ferme SDL_mixer.
 */
void cleanup_audio() {
    // Libère la mémoire occupée par le son
    if (sonEffet != NULL) {
        Mix_FreeChunk(sonEffet);
        sonEffet = NULL;
    }

    // Ferme le sous-système SDL_mixer
    Mix_CloseAudio();
    Mix_Quit();
    printf("Audio nettoyé.\n");
}


// --- Fonction principale de démonstration ---
int main(int argc, char* argv[]) {
    // Initialisation de base de SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Erreur d'initialisation de SDL: %s\n", SDL_GetError());
        return -1;
    }

    // Crée une fenêtre simple pour pouvoir recevoir les événements clavier
    SDL_Window* fenetre = SDL_CreateWindow("Test de Son", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, SDL_WINDOW_SHOWN);
    if (!fenetre) {
        printf("Erreur de création de la fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // --- NOTRE PARTIE AUDIO ---
    // 1. Initialiser l'audio et charger notre son
    if (!init_audio("son.mp3")) {
        // Si l'initialisation échoue, on quitte.
        SDL_DestroyWindow(fenetre);
        SDL_Quit();
        return -1;
    }

    printf("Appuyez sur n'importe quelle touche pour jouer le son. Fermez la fenêtre pour quitter.\n");

    // Boucle d'événements
    SDL_Event e;
    int quitter = 0;
    while (!quitter) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quitter = 1;
            }
            // Si on appuie sur une touche
            if (e.type == SDL_KEYDOWN) {
                printf("Touche pressée ! On joue le son...\n");
                // 2. Appeler la fonction pour jouer le son
                jouer_son();
            }
        }
    }

    // --- NETTOYAGE ---
    // 3. Nettoyer les ressources audio avant de quitter
    cleanup_audio();
    
    SDL_DestroyWindow(fenetre);
    SDL_Quit();

    return 0;
}