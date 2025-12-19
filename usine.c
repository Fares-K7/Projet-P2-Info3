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
                // Pour une source, l'ID est en col 1? Ou col 2? 
                // D'après ton fichier : "Source #..." est souvent en col 2 si c'est la source d'un lien.
                // Adaptons selon la logique : 
                // Si la ligne est "Source -> Plant", la Source est l'émetteur.
                // Note: Dans identifierTypeLigne, si col2 = "Source", c'est TYPE_SOURCE_USINE.
                // Donc l'ID de la source est dans col2 (ID Amont).
                
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
            // On cherche les usines (Plant, Station)
            // Elles apparaissent en destination des Sources (TYPE_SOURCE_USINE)
            // ou en amont des consommateurs. 
            // LE PLUS SIMPLE : Utiliser la définition des usines comme noeuds.
            
            // Si on lit une ligne Source -> Plant, on a une Plant en col3 (Aval)
            if (type == TYPE_SOURCE_USINE) {
                extraireIDAval(&ligne, id);
                extraireVolume(&ligne, &val); // Capacité
                extraireTaux(&ligne, &taux);
                
                insererAVL(avl, id);
                NoeudAVL* n = rechercherUsine(avl, id);
                if (n) {
                    // Pour 'max', on stocke la capacité brute
                    definirVolumeMax(n, val); 
                    
                    // Pour 'real', on calcule : Val * (1 - Taux/100)
                    if (taux < 0) taux = 0;
                    float vReal = val * (1.0 - taux/100.0); // Exemple de calcul "real"
                    ajouterVolumeTraite(n, vReal); // On stocke dans Traite pour le mode 'real'
                }
            }
            // Tu peux ajouter d'autres conditions si les "Plant" apparaissent ailleurs
        }
    }
    fermerFichierCSV(f);

    // --- SORTIE ---
    creerRepertoire("output");
    char cheminSortie[256];
    sprintf(cheminSortie, "output/vol_%s.dat", mode);
    
    FILE* out = fopen(cheminSortie, "w");
    if (!out) { libererAVL(avl); return 2; }
    
    // En-tête imposé par le sujet
    if (strcmp(mode, "max") == 0) fprintf(out, "identifier;max volume (k.m3.year-1)\n");
    else if (strcmp(mode, "src") == 0) fprintf(out, "identifier;source volume (k.m3.year-1)\n");
    else if (strcmp(mode, "real") == 0) fprintf(out, "identifier;real volume (k.m3.year-1)\n");

    // Écriture : il faut adapter ecrireAVLDansFichier dans avl.c pour qu'il sache quelle variable écrire
    // ASTUCE : On passe 'mode' à la fonction d'écriture
    ecrireAVLDansFichier(avl, out, mode);
    
    fclose(out);
    libererAVL(avl);
    return 0;
}
