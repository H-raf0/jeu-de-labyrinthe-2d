void lancer_jeu(int* murs, int lignes, int colonnes) {
    int nb_cellules = lignes * colonnes;

    // --- Initialisation SDL ---
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* fenetre = SDL_CreateWindow("Jeu de Poursuite", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, colonnes * TAILLE_CELLULE, lignes * TAILLE_CELLULE, SDL_WINDOW_SHOWN);
    SDL_Renderer* rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* perso_texture = IMG_LoadTexture(rendu, "personnage.png");
    SDL_Texture* monstre_texture = IMG_LoadTexture(rendu, "monstre.png"); // Assurez-vous d'avoir une image "monstre.png"

    // --- Initialisation des Entités ---
    int joueur_pos = 0;
    Monstre monstres[NOMBRE_MONSTRES];

    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        // Placer les monstres loin du joueur au début
        monstres[i].pos = nb_cellules - 1 - i * colonnes;
        monstres[i].mode = AI_MODE_SEARCH;
        monstres[i].chemin = malloc(sizeof(int) * nb_cellules);
        monstres[i].nb_etapes = 0;
        monstres[i].etape_actuelle = 0;
        monstres[i].plan_cooldown = 0; // Planifie immédiatement
        monstres[i].move_cooldown = 0;
    }

    // --- Boucle de Jeu Principale ---
    bool quitter = false;
    SDL_Event e;

    while (!quitter) {
        // --- 1. GESTION DES ÉVÉNEMENTS (INPUTS) ---
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quitter = true;
            }
            if (e.type == SDL_KEYDOWN) {
                int nouvelle_pos = joueur_pos;
                int direction_flag = 0;

                switch (e.key.keysym.sym) {
                    case SDLK_UP:    nouvelle_pos -= colonnes; direction_flag = 1; break;
                    case SDLK_DOWN:  nouvelle_pos += colonnes; direction_flag = 4; break;
                    case SDLK_LEFT:  nouvelle_pos -= 1;        direction_flag = 8; break;
                    case SDLK_RIGHT: nouvelle_pos += 1;        direction_flag = 2; break;
                }
                // Vérifier si le mouvement est valide (pas de mur)
                if (!(murs[joueur_pos] & direction_flag)) {
                    joueur_pos = nouvelle_pos;
                }
            }
        }

        // --- 2. MISE À JOUR DE LA LOGIQUE (UPDATE) ---
        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
            Monstre *monstre = &monstres[i];

            // -- Mise à jour de l'IA du monstre --
            int j_x, j_y, m_x, m_y;
            indice_vers_coord(joueur_pos, colonnes, &j_x, &j_y);
            indice_vers_coord(monstre->pos, colonnes, &m_x, &m_y);
            int distance = abs(j_x - m_x) + abs(j_y - m_y); // Distance de Manhattan

            // Changer de mode
            int nouveau_mode = (distance < SEUIL_DETECTION) ? AI_MODE_HUNT : AI_MODE_SEARCH;
            if (monstre->mode != nouveau_mode) {
                printf("Monstre %d passe en mode %s\n", i, nouveau_mode == AI_MODE_HUNT ? "CHASSE" : "RECHERCHE");
                monstre->mode = nouveau_mode;
                monstre->plan_cooldown = 0; // Forcer la re-planification
            }
            
            // Re-planifier si le cooldown est terminé
            if (monstre->plan_cooldown <= 0) {
                printf("Monstre %d re-planifie... ", i);
                monstre->plan_cooldown = PLANNING_COOLDOWN;
                
                noeud plan;
                int destination_ia = -1;

                if (monstre->mode == AI_MODE_HUNT) {
                    destination_ia = joueur_pos;
                    printf("cible: joueur en %d\n", destination_ia);
                } else { // AI_MODE_SEARCH
                    // Choisir une destination aléatoire pour patrouiller
                    destination_ia = rand() % nb_cellules;
                    printf("cible: patrouille vers %d\n", destination_ia);
                }
                
                // L'IA utilise A* pour planifier son chemin
                A_etoile_laby(murs, lignes, colonnes, monstre->pos, destination_ia, &plan, HEURISTIC_MANHATTAN);
                monstre->nb_etapes = reconstruire_chemin_inverse(&plan, monstre->pos, destination_ia, nb_cellules, monstre->chemin);
                monstre->etape_actuelle = 0;
                free_noeuds(&plan);

            } else {
                monstre->plan_cooldown--;
            }

            // -- Mouvement du monstre --
            if (monstre->move_cooldown <= 0) {
                if (monstre->etape_actuelle < monstre->nb_etapes - 1) {
                    monstre->etape_actuelle++;
                    monstre->pos = monstre->chemin[monstre->etape_actuelle];
                }
                monstre->move_cooldown = VITESSE_MONSTRE;
            } else {
                monstre->move_cooldown--;
            }

            // Vérifier la condition de défaite
            if (monstre->pos == joueur_pos) {
                printf("GAME OVER ! Le monstre %d vous a attrapé.\n", i);
                quitter = true;
            }
        }

        // --- 3. DESSIN (RENDER) ---
        SDL_SetRenderDrawColor(rendu, 20, 0, 30, 255); // Fond sombre
        SDL_RenderClear(rendu);
        
        // Dessiner les murs
        SDL_SetRenderDrawColor(rendu, 100, 80, 200, 255);
        for (int i = 0; i < nb_cellules; i++) dessiner_murs(rendu, i % colonnes, i / colonnes, murs, colonnes);
        
        // Dessiner le joueur
        dessiner_personnage(rendu, perso_texture, (joueur_pos % colonnes + 0.5f) * TAILLE_CELLULE, (joueur_pos / colonnes + 0.5f) * TAILLE_CELLULE);

        // Dessiner les monstres
        for (int i = 0; i < NOMBRE_MONSTRES; i++) {
             // Change la couleur du monstre selon son mode
            if(monstres[i].mode == AI_MODE_HUNT) SDL_SetTextureColorMod(monstre_texture, 255, 100, 100); // Rouge en mode chasse
            else SDL_SetTextureColorMod(monstre_texture, 100, 255, 100); // Vert en mode recherche
            
            dessiner_personnage(rendu, monstre_texture, (monstres[i].pos % colonnes + 0.5f) * TAILLE_CELLULE, (monstres[i].pos / colonnes + 0.5f) * TAILLE_CELLULE);
        }

        SDL_RenderPresent(rendu);
    }

    // --- Nettoyage ---
    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        free(monstres[i].chemin);
    }
    SDL_DestroyTexture(perso_texture);
    SDL_DestroyTexture(monstre_texture);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
}