void mettre_a_jour_monstre(Monstre* monstres, int monstre_index, int joueur_pos, int* murs_reels, int lignes, int colonnes) {
    Monstre* monstre = &monstres[monstre_index];
    int nb_cellules = lignes * colonnes;

    // --- PERCEPTION & DÉCISION DU MODE (INCHANGÉ) ---
    int j_x, j_y, m_x, m_y;
    indice_vers_coord(joueur_pos, colonnes, &j_x, &j_y);
    indice_vers_coord(monstre->pos, colonnes, &m_x, &m_y);
    int distance = abs(j_x - m_x) + abs(j_y - m_y);
    if (monstre->timer_piste > 0) monstre->timer_piste--;
    int old_mode = monstre->mode;
    if (distance <= SEUIL_DETECTION_HUNT) {
        monstre->mode = AI_MODE_HUNT;
        if (old_mode != AI_MODE_HUNT) printf("Monstre %d -> MODE CHASSE (Détection directe)\n", monstre->pos);
        monstre->drnier_pos_jr_connu = joueur_pos;
        monstre->timer_piste = DUREE_PISTE;
    } 
    else if (monstre->timer_piste > 0) {
        monstre->mode = AI_MODE_HUNT;
        if (old_mode != AI_MODE_HUNT) printf("Monstre %d -> MODE CHASSE (Poursuite de la piste)\n", monstre->pos);
    } 
    else {
        monstre->mode = AI_MODE_SEARCH_ZONE;
        if (old_mode != AI_MODE_SEARCH_ZONE){
            printf("Monstre %d -> MODE RECHERCHE (Piste perdue)\n", monstre->pos);
            monstre->drnier_pos_jr_connu = monstre->pos; 
            monstre->frontier_size = 0;
            memset(monstre->noeuds_visites_zone, 0, sizeof(bool) * (lignes*colonnes));
            monstre->frontier_nodes[monstre->frontier_size++] = monstre->pos;
        }
    }

    if (monstre->move_cooldown > 0) {
        monstre->move_cooldown--;
        return;
    }
    monstre->move_cooldown = VITESSE_MONSTRE;

    // --- NOUVEAU: CRÉATION DE LA CARTE DE PÉNALITÉS ---
    int* penalite_map = calloc(nb_cellules, sizeof(int));
    if (!penalite_map) return; // Sécurité

    for (int i = 0; i < NOMBRE_MONSTRES; i++) {
        if (i == monstre_index) continue; // Ne pas se pénaliser soi-même

        int autre_monstre_pos = monstres[i].pos;
        int am_x, am_y;
        indice_vers_coord(autre_monstre_pos, colonnes, &am_x, &am_y);

        // Appliquer une pénalité dans un rayon autour de l'autre monstre
        for (int y_p = am_y - MONSTRE_PENALITE_RAYON; y_p <= am_y + MONSTRE_PENALITE_RAYON; y_p++) {
            for (int x_p = am_x - MONSTRE_PENALITE_RAYON; x_p <= am_x + MONSTRE_PENALITE_RAYON; x_p++) {
                if (x_p >= 0 && x_p < colonnes && y_p >= 0 && y_p < lignes) {
                    int dist = abs(x_p - am_x) + abs(y_p - am_y);
                    if (dist <= MONSTRE_PENALITE_RAYON) {
                        int penalite = MONSTRE_PENALITE_COUT * (MONSTRE_PENALITE_RAYON - dist) / MONSTRE_PENALITE_RAYON;
                        int cell_idx = y_p * colonnes + x_p;
                        penalite_map[cell_idx] += penalite; // On ajoute, au cas où plusieurs monstres sont proches
                    }
                }
            }
        }
    }

    int prochaine_position_planifiee = monstre->pos;
    
    switch (monstre->mode) {
        case AI_MODE_HUNT:
            // MODIFIÉ: On passe la penalite_map à gidc
            prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, joueur_pos, penalite_map);
            monstre->rapp_cooldown = 0;
            break;
        case AI_MODE_SEARCH_ZONE:
            if (monstre->pos != monstre->drnier_pos_jr_connu && monstre->rapp_cooldown == 0) {
                 // MODIFIÉ: On passe la penalite_map à gidc
                prochaine_position_planifiee = gidc(monstre, murs_reels, lignes, colonnes, monstre->drnier_pos_jr_connu, penalite_map);
                if (prochaine_position_planifiee == monstre->drnier_pos_jr_connu) {
                    monstre->rapp_cooldown = RAPP_CLDWN;
                }
            } else {
                // MODIFIÉ: On passe la penalite_map à gidi
                prochaine_position_planifiee = gidi(monstre, murs_reels, lignes, colonnes, penalite_map);
                if (monstre->rapp_cooldown >= 1){
                    printf("\ncooldown :%d\n", monstre->rapp_cooldown);
                    monstre->rapp_cooldown--;
                }
                if (monstre->rapp_cooldown == 1){
                    monstre->drnier_pos_jr_connu = joueur_pos;
                }
            }
            break;
    }
    
    // NOUVEAU: Libérer la mémoire de la carte de pénalités
    free(penalite_map);

    monstre->pos = prochaine_position_planifiee;
}

    

IGNORE_WHEN_COPYING_START
