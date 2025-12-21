#include "fuites.h"
#include "avl.h"
#include "fichier.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _parcoursFuites(NoeudAVL* n, FILE* out) {
    if (n == NULL) return;

    // 1. Gauche (A)
    _parcoursFuites(n->gauche, out);

    // 2. Traitement
    float fuite = n->volumeCapte - n->volumeTraite;
    if (fuite < 0) fuite = -fuite; // Valeur absolue

    // Conversion en Millions
    float fuiteM = fuite / 1000000.0;
    
    if (fuiteM > 0.0 && (n->volumeCapte > 0 || n->volumeTraite > 0)) {
        fprintf(out, "%s;%.3f\n", n->identifiant, fuiteM);
    }

    // 3. Droite (Z)
    _parcoursFuites(n->droit, out);
}
// ------------------------------------------------------

int traiterFuites(const char* fichierCSV, const char* idRecherche) {
    printf("Analyse des fuites : Mode '%s'...\n", idRecherche);

    // 1. Initialisation
    AVL* avl = creerAVL();
    FILE* f = ouvrirFichierCSV(fichierCSV);
    if (!f) { libererAVL(avl); return 1; }

    LigneCSV ligne;
    char id[TAILLE_CHAMP];
    float val;

    // 2. Remplissage de l'AVL (Ta logique, inchangée)
    while (lireLigneCSV(f, &ligne)) {
        TypeLigneCSV type = identifierTypeLigne(&ligne);

        // CAS 1 : Apport (Ce qui rentre)
        if (type == TYPE_SOURCE_USINE) {
            extraireIDAval(&ligne, id);   
            extraireVolume(&ligne, &val); 
            insererAVL(avl, id);
            NoeudAVL* n = rechercherUsine(avl, id);
            if (n) ajouterVolumeCapte(n, val); 
        }
        // CAS 2 : Consommation (Ce qui sort)
        else if (type == TYPE_RACCORDEMENT_USAGER || type == TYPE_USINE_NOEUD) {
            extraireIDAmont(&ligne, id);  
            extraireVolume(&ligne, &val); 
            insererAVL(avl, id);
            NoeudAVL* n = rechercherUsine(avl, id);
            if (n) ajouterVolumeTraite(n, val); 
        }
    }
    fermerFichierCSV(f);

    // 3. Écriture du fichier de sortie
    creerRepertoire("output");
    FILE* out = fopen("output/leaks.dat", "w");
    if (!out) { libererAVL(avl); return 2; }

    fprintf(out, "identifier;Leak volume (M.m3.year-1)\n");
    
    if (strcmp(idRecherche, "all") == 0) {
        //On parcourt tout l'arbre
        _parcoursFuites(avl->racine, out);
        printf(" -> Export de toutes les fuites terminé.\n");
    }
    else {
        //Une seule usine
        NoeudAVL* cible = rechercherUsine(avl, idRecherche);
        
        if (cible) {
            float entrant = cible->volumeCapte;
            float sortant = cible->volumeTraite;
            float fuite = entrant - sortant;
            
            if (fuite < 0) {
                printf("Info : Fuite négative. On prend la valeur positive.\n");
                fuite = -fuite;
            }

            // Conversion en Millions
            float fuiteM = fuite / 1000000.0;

            fprintf(out, "%s;%.3f\n", idRecherche, fuiteM);
            printf(" -> Usine trouvée. Fuite : %.3f M m3\n", fuiteM);
        } else {
            printf("ERREUR : L'usine '%s' n'existe pas.\n", idRecherche);
            fprintf(out, "%s;0.000\n", idRecherche);
        }
    }

    fclose(out);
    libererAVL(avl);
    return 0;
}
