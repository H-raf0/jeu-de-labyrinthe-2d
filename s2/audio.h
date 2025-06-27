// audio.h

#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <SDL2/SDL_mixer.h>

typedef struct {
    Mix_Music* menuMusic;
    Mix_Music* gameMusic; 
    Mix_Chunk* o2Sound;

    // NOUVEAU : Chunks pour la victoire et la défaite
    Mix_Chunk* victorySound;
    Mix_Chunk* failureSound;
} AudioData;

bool init_audio_system();
void load_sounds(AudioData* audio);
void start_menu_music(const AudioData* audio);
void start_game_music(const AudioData* audio);
void play_o2_sound(const AudioData* audio);

// NOUVEAU : Fonctions pour jouer les sons de fin de partie
void play_victory_sound(const AudioData* audio);
void play_failure_sound(const AudioData* audio);

void toggle_music(bool sound_on);
void cleanup_audio(AudioData* audio);

#endif // AUDIO_H