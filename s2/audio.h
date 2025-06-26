#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <SDL2/SDL_mixer.h>

// Structure pour contenir les données audio
typedef struct {
    // Renommé pour plus de clarté et ajout de la musique du jeu
    Mix_Music* menuMusic;
    Mix_Music* gameMusic; 
} AudioData;

// Initialise le système audio de SDL_mixer
bool init_audio_system();

// Charge les fichiers sonores
void load_sounds(AudioData* audio);

// --- NOUVELLES FONCTIONS ---
// Démarre la musique du menu
void start_menu_music(const AudioData* audio);
// Démarre la musique du jeu
void start_game_music(const AudioData* audio);
// -------------------------

// Gère la mise en pause/reprise de la musique
void toggle_music(bool sound_on);

// Libère les ressources audio
void cleanup_audio(AudioData* audio);

#endif // AUDIO_H