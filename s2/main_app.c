#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "drawing.h"
#include "audio.h"
#include "laby.h"
#include "jeu.h"
//#include "endscreen.h"

// Note : g_config est une variable globale définie dans labySDL.c
// et déclarée comme 'extern' via labySDL.h (inclus par jeu.h ou laby.h)
// Elle contiendra les dimensions de l'écran.


int LABY_HE = 20;
int LABY_WI = 30;


// La signature de la fonction ne change pas, car elle utilisera g_config
void show_main_menu(SDL_Window* window, SDL_Renderer* renderer) {
    // --- Chargement des ressources du menu ---
    AudioData audio_data;
    load_sounds(&audio_data);

    start_menu_music(&audio_data);


    ParallaxBackground background;
    // On passe les dimensions de l'écran, stockées dans g_config, à init_parallax
    init_parallax(renderer, &background, g_config.window_w);

    // --- Variables du menu ---
    const char* difficulty_names[] = {"Facile", "Moyen", "Difficile"};
    DifficultyLevel current_difficulty = EASY;
    bool sound_on = true;

    // --- Calcul de la disposition relative du menu ---
    int button_width = g_config.window_w / 3.5;  // Largeur relative à la taille de l'écran
    int button_height = g_config.window_h / 12; // Hauteur relative à la taille de l'écran
    int center_x = (g_config.window_w - button_width) / 2;
    int spacing = button_height * 1.5; // Espace entre les boutons, également relatif

    // Positionnement vertical centré et relatif
    int start_y = g_config.window_h / 2 - (int)(spacing * 1.5);

    SDL_Rect play_rect = {center_x, start_y, button_width, button_height};
    SDL_Rect diff_rect = {center_x, start_y + spacing, button_width, button_height};
    SDL_Rect quit_rect = {center_x, start_y + 2 * spacing, button_width, button_height};
    
    // Position du bouton son en haut à droite, de taille relative
    int sound_button_size = g_config.window_w / 30;
    if (sound_button_size < 30) sound_button_size = 30; // Taille minimale
    SDL_Rect sound_rect = {g_config.window_w - sound_button_size - 20, 20, sound_button_size, sound_button_size};
    
    // Rectangles pour les flèches de difficulté
    SDL_Rect diff_left_arrow_rect = {diff_rect.x, diff_rect.y, diff_rect.w / 3, diff_rect.h};
    SDL_Rect diff_right_arrow_rect = {diff_rect.x + (2 * diff_rect.w / 3), diff_rect.y, diff_rect.w / 3, diff_rect.h};

    // --- Boucle du Menu ---
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                // Pour quitter l'application, il faut sortir de la boucle du menu
                running = false; 
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                
                SDL_Point mouse_pos = {event.button.x, event.button.y};

                if (SDL_PointInRect(&mouse_pos, &play_rect)) {
                    printf("Lancement du jeu niveau %s !\n", difficulty_names[current_difficulty]);


                    switch(current_difficulty) {
                        default:
                        case EASY:
                            LABY_HE = 10;
                            LABY_WI = 15;
                            NOMBRE_MONSTRES = 2;
                            SEUIL_DETECTION_HUNT = 2;
                            DUREE_PISTE = 10;
                            RAPP_CLDWN = 200;
                            MEMOIRE_MAX = 30;
                            VITESSE_MONSTRE = 10;
                            MONSTRE_PENALITE_RAYON = 4;
                            MONSTRE_PENALITE_COUT = 20;

                            SAUT_COOLDOWN = 10; // Cooldown en frames

                            NOMBRE_PIECES = 1;
                            break;
                        case MEDIUM:
                            LABY_HE = 20;
                            LABY_WI = 25;
                            NOMBRE_MONSTRES = 3;
                            SEUIL_DETECTION_HUNT = 5;
                            DUREE_PISTE = 50;
                            RAPP_CLDWN = 80;
                            MEMOIRE_MAX = 200;
                            VITESSE_MONSTRE = 0;
                            MONSTRE_PENALITE_RAYON = 4;
                            MONSTRE_PENALITE_COUT = 20;

                            SAUT_COOLDOWN = 100; // Cooldown en frames

                            NOMBRE_PIECES = 2;
                            break;
                        case HARD:
                            LABY_HE = 30;
                            LABY_WI = 40;
                            NOMBRE_MONSTRES = 5;
                            SEUIL_DETECTION_HUNT = 10;
                            DUREE_PISTE = 100;
                            RAPP_CLDWN = 50;
                            MEMOIRE_MAX = 99999;
                            VITESSE_MONSTRE = 0;
                            MONSTRE_PENALITE_RAYON = 5;
                            MONSTRE_PENALITE_COUT = 25;

                            SAUT_COOLDOWN = 500; // Cooldown en frames

                            NOMBRE_PIECES = 3;
                            break;
                    }

                    
                    // 2. GÉNÉRER LE LABYRINTHE MAINTENANT, AVEC LES BONNES DIMENSIONS
                    int lignes = LABY_HE;
                    int colonnes = LABY_WI;
                    int nb_cellules = lignes * colonnes;
                    
                    arete *toutes_aretes;
                    int nb_total_aretes = generation_grille_vide(&toutes_aretes, lignes, colonnes);
                    fisher_yates(toutes_aretes, nb_total_aretes);
                    
                    arete *arbre = malloc(sizeof(arete) * (nb_cellules - 1));
                    int nb_aretes_arbre;
                    construire_arbre_couvrant(toutes_aretes, nb_total_aretes, arbre, &nb_aretes_arbre, nb_cellules);
                    free(toutes_aretes);
                    
                    int *murs_reels = malloc(sizeof(int) * nb_cellules);
                    for (int i = 0; i < nb_cellules; i++) murs_reels[i] = 1 | 2 | 4 | 8;
                    for (int i = 0; i < nb_aretes_arbre; i++) supprimer_mur(murs_reels, colonnes, arbre[i].u, arbre[i].v);
                    free(arbre);


                    start_game_music(&audio_data);
                    GameResult result = lancer_jeu(renderer, murs_reels, lignes, colonnes, &audio_data);
                    

                    free(murs_reels);


                    // NOUVEAU : On agit en fonction du résultat
                    switch(result) {
                        case GAME_WON:
                            show_end_screen(renderer, "congratulations.bmp"); // Utilisez PNG pour la transparence
                            break;
                        case GAME_LOST:
                            show_end_screen(renderer, "gameover.bmp");
                            break;
                        case GAME_QUIT_MANUALLY:
                            // Ne rien faire, on retourne juste au menu
                            break;
                    }
                    
                    start_menu_music(&audio_data);
                    
                } else if (SDL_PointInRect(&mouse_pos, &diff_left_arrow_rect)) {
                    current_difficulty = (current_difficulty - 1 + DIFFICULTY_COUNT) % DIFFICULTY_COUNT;
                    printf("Changement de difficulté -> Niveau %s\n", difficulty_names[current_difficulty]);
                    
                } else if (SDL_PointInRect(&mouse_pos, &diff_right_arrow_rect)) {
                    current_difficulty = (current_difficulty + 1) % DIFFICULTY_COUNT;
                    printf("Changement de difficulté -> Niveau %s\n", difficulty_names[current_difficulty]);
                    
                } else if (SDL_PointInRect(&mouse_pos, &quit_rect)) {
                    running = false;
                    
                } else if (SDL_PointInRect(&mouse_pos, &sound_rect)) {
                    // Le son a déjà été joué, on ne fait que changer l'état
                    sound_on = !sound_on;
                    toggle_music(sound_on);
                }
                
                
            }
        }

        // --- Rendu du menu ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // On passe les dimensions de l'écran, stockées dans g_config, à draw_parallax
        draw_parallax(renderer, &background, g_config.window_w, g_config.window_h);

        SDL_Point mouse_pos;
        SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
        bool is_hovering_play = SDL_PointInRect(&mouse_pos, &play_rect);
        bool is_hovering_diff = SDL_PointInRect(&mouse_pos, &diff_rect);
        bool is_hovering_quit = SDL_PointInRect(&mouse_pos, &quit_rect);
        bool is_hovering_sound = SDL_PointInRect(&mouse_pos, &sound_rect);
        
        draw_decorated_button(renderer, &play_rect, "play", current_difficulty, is_hovering_play);
        draw_decorated_button(renderer, &diff_rect, "difficulty", current_difficulty, is_hovering_diff);
        draw_decorated_button(renderer, &quit_rect, "quit", current_difficulty, is_hovering_quit);
        draw_sound_icon(renderer, &sound_rect, sound_on, is_hovering_sound);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // 60fps
    }

    // --- Nettoyage des ressources du menu ---
    cleanup_drawing(&background);
    cleanup_audio(&audio_data);
}


int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    // --- Initialisation Globale de SDL ---
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image initialization failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    if (!init_audio_system()) {
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // --- Création Fenêtre et Rendu directement en plein écran ---
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        printf("SDL_GetDesktopDisplayMode failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Labyrinthe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, dm.w, dm.h, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return 1;
    }

    // --- Initialisation de la configuration globale de rendu ---
    // Cette configuration sera utilisée par le menu ET par le jeu
    g_config.window_w = dm.w;
    g_config.window_h = dm.h;

    // --- Génération du Labyrinthe (logique de l'ancien main de jeu.c) ---
    srand(time(NULL));
    

    // --- Lancement du Menu ---
    // La fenêtre est déjà en plein écran, et g_config est prête
    show_main_menu(window, renderer);

    // --- Nettoyage Final ---
    printf("Fermeture du programme.\n");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}