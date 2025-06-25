// dans jeu.c

// Fonction de jeu principale
void lancer_jeu(int* murs_reels, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* fenetre = SDL_CreateWindow("Jeu IA Focalisée", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, colonnes * TAILLE_CELLULE, lignes * TAILLE_CELLULE, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* perso_texture = IMG_LoadTexture(rendu, "personnage.png");
    SDL_Texture* monstre_texture = IMG_LoadTexture(rendu, "monstre.png");

    // Initialisation du Joueur
    Joueur joueur;
    joueur.pos = 0;
    joueur.direction = 4; // Commence en regardant vers le bas
    joueur.saut_cooldown = 0;

    // Initialisation des Monstres
    Monstre monstres[NOMBRE_MONSTRES];
    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        Monstre* m = &monstres[i];
        m->pos = nb_cellules - 1 - i * 2;
        m->mode = AI_MODE_SEARCH_ZONE;
        m->timer_piste = 0;
        m->murs_connus = calloc(nb_cellules, sizeof(int));
        m->memoire_murs = malloc(sizeof(arete) * MEMOIRE_MAX);
        m->memoire_tete = 0; m->memoire_queue = 0; m->memoire_taille_actuelle = 0;
        m->noeuds_visites_zone = calloc(nb_cellules, sizeof(bool));
        m->frontier_nodes = malloc(sizeof(int) * nb_cellules);
        m->frontier_size = 0;
        m->prochaine_position = m->pos;
        m->drnier_pos_jr_connu = joueur.pos; 
        m->rapp_cooldown = RAPP_CLDWN;
        m->move_cooldown = 1;
    }

    bool quitter = false;
    SDL_Event e;
    while (!quitter) {
        // Décrémenter le cooldown du saut du joueur à chaque frame
        if (joueur.saut_cooldown > 0) {
            joueur.saut_cooldown--;
        }

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quitter = true;
            }
            if (e.type == SDL_KEYDOWN) {
                int nouvelle_pos = joueur.pos;
                int direction_flag = 0;

                switch (e.key.keysym.sym) {
                    // Mouvement normal
                    case SDLK_UP:    direction_flag = 1; nouvelle_pos -= colonnes; break;
                    case SDLK_DOWN:  direction_flag = 4; nouvelle_pos += colonnes; break;
                    case SDLK_LEFT:  direction_flag = 8; nouvelle_pos -= 1;        break;
                    case SDLK_RIGHT: direction_flag = 2; nouvelle_pos += 1;        break;

                    // --- NOUVELLE LOGIQUE DE SAUT ---
                    case TOUCHE_SAUT:
                        // On ne peut sauter que si le cooldown est terminé
                        if (joueur.saut_cooldown == 0) {
                            int pos_apres_saut = -1;
                            // On vérifie s'il y a bien un mur dans la direction où le joueur regarde
                            if (murs_reels[joueur.pos] & joueur.direction) {
                                // Calculer la position de l'autre côté du mur
                                if (joueur.direction == 1) pos_apres_saut = joueur.pos - colonnes; // HAUT
                                else if (joueur.direction == 4) pos_apres_saut = joueur.pos + colonnes; // BAS
                                else if (joueur.direction == 8) pos_apres_saut = joueur.pos - 1;       // GAUCHE
                                else if (joueur.direction == 2) pos_apres_saut = joueur.pos + 1;       // DROITE
                                
                                // Vérifier que la case d'atterrissage est valide (dans le labyrinthe)
                                int x, y;
                                indice_vers_coord(pos_apres_saut, colonnes, &x, &y);
                                if (x >= 0 && x < colonnes && y >= 0 && y < lignes) {
                                    printf("Le joueur a sauté par-dessus un mur !\n");
                                    joueur.pos = pos_apres_saut;
                                    joueur.saut_cooldown = SAUT_COOLDOWN; // Activer le cooldown
                                }
                            }
                        }
                        // On met le flag à 0 pour ne pas entrer dans la logique de marche normale
                        direction_flag = 0;
                        break;
                }

                // Si une touche de direction a été pressée
                if (direction_flag != 0) {
                    // On met à jour la direction dans laquelle le joueur regarde
                    joueur.direction = direction_flag;
                    // On vérifie si le mouvement est valide (pas de mur)
                    if (!(murs_reels[joueur.pos] & direction_flag)) {
                        joueur.pos = nouvelle_pos;
                    }
                }
            }
        }

        // Mise à jour des monstres
        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            mettre_a_jour_monstre(&monstres[i], joueur.pos, murs_reels, lignes, colonnes);
            if (monstres[i].pos == joueur.pos) { printf("GAME OVER !\n"); quitter = true; }
        }

        // --- DESSIN ---
        SDL_SetRenderDrawColor(rendu, 20, 0, 30, 255);
        SDL_RenderClear(rendu);
        SDL_SetRenderDrawColor(rendu, 100, 80, 200, 255);
        for (int i = 0; i < nb_cellules; i++) dessiner_murs(rendu, i % colonnes, i / colonnes, murs_reels, colonnes);
        
        dessiner_personnage(rendu, perso_texture, (joueur.pos % colonnes + 0.5f) * TAILLE_CELLULE, (joueur.pos / colonnes + 0.5f) * TAILLE_CELLULE);
        
        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            if(monstres[i].mode == AI_MODE_HUNT) SDL_SetTextureColorMod(monstre_texture, 255, 100, 100);
            else SDL_SetTextureColorMod(monstre_texture, 255, 255, 100);
            dessiner_personnage(rendu, monstre_texture, (monstres[i].pos % colonnes + 0.5f) * TAILLE_CELLULE, (monstres[i].pos / colonnes + 0.5f) * TAILLE_CELLULE);
        }
        SDL_RenderPresent(rendu);
    }

    // Libération de la mémoire
    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        free(monstres[i].murs_connus);
        free(monstres[i].memoire_murs);
        free(monstres[i].noeuds_visites_zone);
        free(monstres[i].frontier_nodes);
    }
    SDL_DestroyTexture(perso_texture);
    SDL_DestroyTexture(monstre_texture);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre); 
    SDL_Quit();
}