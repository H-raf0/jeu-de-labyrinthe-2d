// audio.c

#include "audio.h"
#include <stdio.h>

// ... init_audio_system() est inchangé ...
bool init_audio_system() { if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { printf("SDL_mixer initialization failed: %s\n", Mix_GetError()); return false; } Mix_AllocateChannels(16); Mix_Volume(-1, MIX_MAX_VOLUME); return true; }

void load_sounds(AudioData* audio) {
    audio->menuMusic = Mix_LoadMUS("menu_music.mp3");
    if (!audio->menuMusic) { /* ... */ }

    audio->gameMusic = Mix_LoadMUS("game_music.mp3");
    if (!audio->gameMusic) { /* ... */ }

    audio->o2Sound = Mix_LoadWAV("o2_collect.wav");
    if (!audio->o2Sound) { /* ... */ }

    // NOUVEAU : Charger les sons de victoire et de défaite
    audio->victorySound = Mix_LoadWAV("victory.wav");
    if (!audio->victorySound) {
        printf("Failed to load victory sound: %s\n", Mix_GetError());
    }

    audio->failureSound = Mix_LoadWAV("failure.wav");
    if (!audio->failureSound) {
        printf("Failed to load failure sound: %s\n", Mix_GetError());
    }
}

// ... les fonctions play_o2_sound, start_menu_music, start_game_music sont inchangées ...
void play_o2_sound(const AudioData* audio) { if (audio->o2Sound) { if (Mix_PlayChannel(-1, audio->o2Sound, 0) == -1) { printf("Failed to play o2 sound: %s\n", Mix_GetError()); } } }
void start_menu_music(const AudioData* audio) { Mix_HaltMusic(); if (audio->menuMusic) { if (Mix_PlayMusic(audio->menuMusic, -1) == -1) { printf("Failed to play menu music: %s\n", Mix_GetError()); } } }
void start_game_music(const AudioData* audio) { Mix_HaltMusic(); if (audio->gameMusic) { if (Mix_PlayMusic(audio->gameMusic, -1) == -1) { printf("Failed to play game music: %s\n", Mix_GetError()); } } }


// NOUVEAU : Implémentation des fonctions pour jouer les sons de fin
void play_victory_sound(const AudioData* audio) {
    if (audio->victorySound) {
        Mix_PlayChannel(-1, audio->victorySound, 0);
    }
}

void play_failure_sound(const AudioData* audio) {
    if (audio->failureSound) {
        Mix_PlayChannel(-1, audio->failureSound, 0);
    }
}

// ... toggle_music() est inchangé ...
void toggle_music(bool sound_on) { if (sound_on) { Mix_ResumeMusic(); } else { Mix_PauseMusic(); } }

void cleanup_audio(AudioData* audio) {
    if (audio->menuMusic) { Mix_FreeMusic(audio->menuMusic); }
    if (audio->gameMusic) { Mix_FreeMusic(audio->gameMusic); }
    if (audio->o2Sound) { Mix_FreeChunk(audio->o2Sound); }

    // NOUVEAU : Libérer les nouveaux chunks
    if (audio->victorySound) {
        Mix_FreeChunk(audio->victorySound);
    }
    if (audio->failureSound) {
        Mix_FreeChunk(audio->failureSound);
    }
    
    Mix_CloseAudio();
}