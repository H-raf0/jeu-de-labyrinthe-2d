#include "audio.h"
#include <stdio.h>

bool init_audio_system() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer initialization failed: %s\n", Mix_GetError());
        return false;
    }
    Mix_AllocateChannels(16);
    Mix_Volume(-1, MIX_MAX_VOLUME);
    return true;
}

void load_sounds(AudioData* audio) {
    // Charger la musique du menu
    audio->menuMusic = Mix_LoadMUS("menu_music.mp3");
    if (!audio->menuMusic) {
        printf("Failed to load menu music: %s\n", Mix_GetError());
    }

    // Charger la musique du jeu
    audio->gameMusic = Mix_LoadMUS("game_music.mp3"); // Assurez-vous que ce fichier existe
    if (!audio->gameMusic) {
        printf("Failed to load game music: %s\n", Mix_GetError());
    }
}

// --- IMPLÉMENTATION DES NOUVELLES FONCTIONS ---
void start_menu_music(const AudioData* audio) {
    Mix_HaltMusic(); // Arrête toute musique en cours
    if (audio->menuMusic) {
        if (Mix_PlayMusic(audio->menuMusic, -1) == -1) {
            printf("Failed to play menu music: %s\n", Mix_GetError());
        }
    }
}

void start_game_music(const AudioData* audio) {
    Mix_HaltMusic(); // Arrête toute musique en cours
    if (audio->gameMusic) {
        if (Mix_PlayMusic(audio->gameMusic, -1) == -1) {
            printf("Failed to play game music: %s\n", Mix_GetError());
        }
    }
}
// ---------------------------------------------

// Cette fonction met en pause/reprend la musique en cours, quelle qu'elle soit.
void toggle_music(bool sound_on) {
    if (sound_on) {
        Mix_ResumeMusic();
    } else {
        Mix_PauseMusic();
    }
}

void cleanup_audio(AudioData* audio) {
    if (audio->menuMusic) {
        Mix_FreeMusic(audio->menuMusic);
    }
    // Ne pas oublier de libérer la nouvelle piste musicale
    if (audio->gameMusic) {
        Mix_FreeMusic(audio->gameMusic);
    }
    Mix_CloseAudio();
}