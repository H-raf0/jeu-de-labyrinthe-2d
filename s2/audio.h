#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <SDL2/SDL_mixer.h>

// Structure pour contenir les données audio
typedef struct {
    Mix_Chunk* clickSound;
    Mix_Music* backgroundMusic;
} AudioData;

// Initialise le système audio de SDL_mixer
bool init_audio_system();

// Charge les fichiers sonores
void load_sounds(AudioData* audio);

// Joue le son de clic
void play_click_sound(const AudioData* audio);

// Gère la mise en pause/reprise de la musique
void toggle_music(bool sound_on);

// Libère les ressources audio
void cleanup_audio(AudioData* audio);

#endif // AUDIO_H