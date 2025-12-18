#include "usine.h"
#include "avl.h"
#include "fichier.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>

int traiterHistogramme(const char* fichierCSV, const char* mode) {
    AVL* avl = creerAVL();
    FILE* f = ouvrirFichierCSV(fichierCSV);
    if (!f) { libererAVL(avl); return 1; }

    LigneCSV ligne;
    char id[TAILLE_CHAMP];
    float val, val2;

    while (lireLigneCSV(f, &ligne)) {
        TypeLigneCSV type = identifierTypeLigne(&ligne);
        
        if (type == TYPE_SOURCE_USINE) {
            extraireIDAval(&ligne, id);
            extraireVolume(&ligne, &val);
            extraireTaux(&ligne, &val2); 
            
            insererAVL(avl, id);
            NoeudAVL* n = rechercherUsine(avl, id);
            if (n) {
                ajouterVolumeCapte(n, val);
                if (val2 < 0) val2 = 0;
                float vTraite = val * (1.0 - val2/100.0);
                ajouterVolumeTraite(n, vTraite);
            }
        }
        else if (type == TYPE_USINE_NOEUD) {
            extraireIDAmont(&ligne, id);
            extraireVolume(&ligne, &val); 
            insererAVL(avl, id);
            NoeudAVL* n = rechercherUsine(avl, id);
            if (n) definirVolumeMax(n, val);
        }
    }
    fermerFichierCSV(f);

    creerRepertoire("output");
    char cheminSortie[256];
    sprintf(cheminSortie, "output/vol_%s.dat", mode);
    
    FILE* out = fopen(cheminSortie, "w");
    if (!out) { libererAVL(avl); return 2; }
    
    if (strcmp(mode, "max")==0) fprintf(out, "identifier;max volume (k.m3.year-1)\n");
    else if (strcmp(mode, "src")==0) fprintf(out, "identifier;source volume (k.m3.year-1)\n");
    else fprintf(out, "identifier;real volume (k.m3.year-1)\n");

    ecrireAVLDansFichier(avl, out, mode);
    fclose(out);
    libererAVL(avl);
    return 0;
}
