#include "audio.h"
#include <stdio.h>

bool init_audio_system() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer initialization failed: %s\n", Mix_GetError());
        return false;
    }
    return true;
}

void load_sounds(AudioData* audio) {
    // Charger le son de clic
    audio->clickSound = Mix_LoadWAV("click.wav");
    if (!audio->clickSound) {
        printf("Failed to load click sound: %s\n", Mix_GetError());
    }

    // Charger la musique de fond
    audio->backgroundMusic = Mix_LoadMUS("music.mp3");
    if (audio->backgroundMusic) {
        if (Mix_PlayMusic(audio->backgroundMusic, -1) == -1) {
            printf("Failed to play music: %s\n", Mix_GetError());
        }
    } else {
        printf("Failed to load music: %s\n", Mix_GetError());
    }
}

void play_click_sound(const AudioData* audio) {
    if (audio->clickSound) {
        Mix_PlayChannel(-1, audio->clickSound, 0);
    }
}

void toggle_music(bool sound_on) {
    if (sound_on) {
        Mix_ResumeMusic();
        // Rétablit le volume pour tous les canaux
        Mix_Volume(-1, MIX_MAX_VOLUME); 
        Mix_VolumeMusic(MIX_MAX_VOLUME);
    } else {
        Mix_PauseMusic();
        // Met le volume à 0 pour tous les canaux
        Mix_Volume(-1, 0);
        Mix_VolumeMusic(0);
    }
}

void cleanup_audio(AudioData* audio) {
    if (audio->clickSound) {
        Mix_FreeChunk(audio->clickSound);
    }
    if (audio->backgroundMusic) {
        Mix_FreeMusic(audio->backgroundMusic);
    }
    Mix_CloseAudio();
}