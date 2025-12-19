#include "fuites.h"
#include "avl.h"
#include "fichier.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Cette fonction remplace ton approche récursive par une approche AVL "Bilan"
// C'est beaucoup plus simple car on a déjà stocké les sommes dans l'AVL.

int traiterFuites(const char* fichierCSV, const char* idRecherche) {
    printf("Analyse des fuites pour l'usine : '%s'...\n", idRecherche);

    // 1. Initialisation
    AVL* avl = creerAVL();
    FILE* f = ouvrirFichierCSV(fichierCSV);
    if (!f) { libererAVL(avl); return 1; }

    LigneCSV ligne;
    char id[TAILLE_CHAMP];
    float val;

    // 2. Remplissage de l'AVL
    // On parcourt tout le fichier pour additionner :
    // - Tout ce qui RENTRE dans une station (Source -> Station)
    // - Tout ce qui SORT d'une station (Station -> Client)
    while (lireLigneCSV(f, &ligne)) {
        TypeLigneCSV type = identifierTypeLigne(&ligne);

        // CAS 1 : Apport (Ce qui rentre)
        if (type == TYPE_SOURCE_USINE) {
            extraireIDAval(&ligne, id);   // ID de la Station (Destination)
            extraireVolume(&ligne, &val); // Volume
            
            insererAVL(avl, id);
            NoeudAVL* n = rechercherUsine(avl, id);
            // On stocke le TOTAL ENTRANT dans volumeCapte
            if (n) ajouterVolumeCapte(n, val); 
        }

        // CAS 2 : Consommation (Ce qui sort)
        else if (type == TYPE_RACCORDEMENT_USAGER || type == TYPE_USINE_NOEUD) {
            extraireIDAmont(&ligne, id);  // ID de la Station (Source du lien)
            extraireVolume(&ligne, &val); // Volume
            
            insererAVL(avl, id);
            NoeudAVL* n = rechercherUsine(avl, id);
            // On stocke le TOTAL SORTANT dans volumeTraite
            if (n) ajouterVolumeTraite(n, val); 
        }
    }
    fermerFichierCSV(f);

    // 3. Recherche et Calcul
    NoeudAVL* cible = rechercherUsine(avl, idRecherche);

    // 4. Écriture du fichier de sortie
    creerRepertoire("output");
    FILE* out = fopen("output/leaks.dat", "w");
    if (!out) { libererAVL(avl); return 2; }

    fprintf(out, "identifier;Leak volume (M.m3.year-1)\n");

    if (cible) {
        // LE CALCUL DE LA FUITE EST ICI :
        float entrant = cible->volumeCapte;
        float sortant = cible->volumeTraite;
        float fuite = entrant - sortant;
        
        // Si la fuite est négative (impossible physiquement mais possible si données bizarres), on met 0 ou on laisse
        // Pour l'affichage, on garde la valeur brute.
        if (fuite < 0) {
            printf("Info : Fuite négative (%.2f). On prend la valeur positive.\n", fuite);
            fuite = -fuite; // ou fabsf(fuite)
        }
        fprintf(out, "%s;%.1f\n", idRecherche, fuite);
        printf(" -> Usine trouvée. Entrée: %.1f | Sortie: %.1f | Fuite: %.1f\n", entrant, sortant, fuite);
    } else {
        printf("ERREUR : L'usine '%s' n'existe pas dans le fichier.\n", idRecherche);
        fprintf(out, "%s;0\n", idRecherche);
    }

    fclose(out);
    libererAVL(avl);
    return 0;
}
