#include "usine.h"
#include "avl.h"
#include "fichier.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int traiterHistogramme(const char* fichierCSV, const char* mode) {
    AVL* avl = creerAVL();
    FILE* f = ouvrirFichierCSV(fichierCSV);
    if (!f) { libererAVL(avl); return 1; }

    LigneCSV ligne;
    char id[TAILLE_CHAMP];
    float val, taux;

    while (lireLigneCSV(f, &ligne)) {
        TypeLigneCSV type = identifierTypeLigne(&ligne);
        
        // --- CAS 1 : On demande les SOURCES ('src') ---
        if (strcmp(mode, "src") == 0) {
            // On ne garde que les lignes de type SOURCE
            if (type == TYPE_SOURCE_USINE) { 
                
                extraireIDAmont(&ligne, id); 
                extraireVolume(&ligne, &val); // Volume produit
                
                // On insère ou cumule
                insererAVL(avl, id);
                NoeudAVL* n = rechercherUsine(avl, id);
                if (n) ajouterVolumeCapte(n, val); // On utilise VolumeCapte pour stocker la valeur
            }
        }

        // --- CAS 2 & 3 : On demande les USINES ('max' ou 'real') ---
        else if (strcmp(mode, "max") == 0 || strcmp(mode, "real") == 0) {
            if (type == TYPE_SOURCE_USINE) {
                extraireIDAval(&ligne, id);
                extraireVolume(&ligne, &val); 
                extraireTaux(&ligne, &taux);
                
                insererAVL(avl, id);
                NoeudAVL* n = rechercherUsine(avl, id);
                if (n) {
                    
                    definirVolumeMax(n, val); 
                    
                    if (taux < 0) taux = 0;
                    float vReal = val * (1.0 - taux/100.0);
                    ajouterVolumeTraite(n, vReal); 
                }
            }
           
        }
    }
    fermerFichierCSV(f);

    // --- SORTIE ---
    creerRepertoire("output");
    char cheminSortie[256];
    sprintf(cheminSortie, "output/vol_%s.dat", mode);
    
    FILE* out = fopen(cheminSortie, "w");
    if (!out) { libererAVL(avl); return 2; }
    
    if (strcmp(mode, "max") == 0) fprintf(out, "identifier;max volume (k.m3.year-1)\n");
    else if (strcmp(mode, "src") == 0) fprintf(out, "identifier;source volume (k.m3.year-1)\n");
    else if (strcmp(mode, "real") == 0) fprintf(out, "identifier;real volume (k.m3.year-1)\n");

    ecrireAVLDansFichier_Inverse(avl, out, mode);
    
    fclose(out);
    libererAVL(avl);
    return 0;
}
