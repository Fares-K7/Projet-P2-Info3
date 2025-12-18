#include "fuites.h"
#include "arbre.h"
#include "fichier.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Structure AVL interne pour indexer les noeuds de l'arbre N-aire
// Permet de retrouver le parent en O(log n)
typedef struct AVLRef {
    char id[100];
    Noeud* ptr;
    struct AVLRef *g, *d;
} AVLRef;

static AVLRef* insRef(AVLRef* r, const char* id, Noeud* ptr) {
    if (!r) {
        AVLRef* n = malloc(sizeof(AVLRef));
        strcpy(n->id, id); n->ptr = ptr; n->g = n->d = NULL;
        return n;
    }
    int cmp = strcmp(id, r->id);
    if (cmp < 0) r->g = insRef(r->g, id, ptr);
    else if (cmp > 0) r->d = insRef(r->d, id, ptr);
    return r;
}

static Noeud* findRef(AVLRef* r, const char* id) {
    while (r) {
        int cmp = strcmp(id, r->id);
        if (cmp == 0) return r->ptr;
        if (cmp < 0) r = r->g; else r = r->d;
    }
    return NULL;
}

static void freeRef(AVLRef* r) {
    if (r) { freeRef(r->g); freeRef(r->d); free(r); }
}

static double calculerFuitesRec(Noeud* n, double volEntrant, double* totalFuites) {
    if (!n) return 0;
    double fuiteIci = volEntrant * (n->taux / 100.0);
    *totalFuites += fuiteIci;
    double volRestant = volEntrant - fuiteIci;
    
    if (n->nbEnfants > 0) {
        double volParEnfant = volRestant / n->nbEnfants;
        for (int i=0; i<n->nbEnfants; i++) {
            calculerFuitesRec(n->enfants[i], volParEnfant, totalFuites);
        }
    }
    return fuiteIci;
}

int traiterFuites(const char* fichierCSV, const char* idUsine) {
    printf("Traitement fuites pour %s...\n", idUsine);
    FILE* f = ouvrirFichierCSV(fichierCSV);
    if (!f) return 1;

    Noeud* racine = creerNoeud(idUsine);
    AVLRef* index = NULL;
    index = insRef(index, idUsine, racine);

    LigneCSV ligne;
    char idAmont[TAILLE_CHAMP], idAval[TAILLE_CHAMP];
    float taux;

    // Phase 1 : Construction de l'arbre
    while (lireLigneCSV(f, &ligne)) {
        TypeLigneCSV type = identifierTypeLigne(&ligne);
        
        if (type == TYPE_USINE_STOCKAGE || type == TYPE_STOCKAGE_JONCTION ||
            type == TYPE_JONCTION_RACCORDEMENT || type == TYPE_RACCORDEMENT_USAGER) {
            
            extraireIDAmont(&ligne, idAmont);
            Noeud* parent = findRef(index, idAmont);
            
            if (parent) {
                extraireIDAval(&ligne, idAval);
                extraireTaux(&ligne, &taux);
                Noeud* enfant = creerNoeud(idAval);
                enfant->taux = taux;
                ajouterEnfant(parent, enfant);
                index = insRef(index, idAval, enfant);
            }
        }
    }
    
    // Phase 2 : Calcul du volume initial
    rewind(f);
    double volumeInitial = 0;
    float vol, tx;
    while (lireLigneCSV(f, &ligne)) {
        if (identifierTypeLigne(&ligne) == TYPE_SOURCE_USINE) {
            extraireIDAval(&ligne, idAval);
            if (strcmp(idAval, idUsine) == 0) {
                extraireVolume(&ligne, &vol);
                extraireTaux(&ligne, &tx);
                if (tx < 0) tx = 0;
                volumeInitial += vol * (1.0 - tx/100.0);
            }
        }
    }
    fermerFichierCSV(f);

    // Phase 3 : Calcul final
    double totalFuites = 0;
    calculerFuitesRec(racine, volumeInitial, &totalFuites);

    creerRepertoire("output");
    FILE* out = fopen("output/rendement.dat", "w");
    if (out) {
        fprintf(out, "identifier;leak volume (M.m3.year-1)\n");
        // Sortie brute (k.m3 ou M.m3 selon unité d'entrée, ici on garde l'unité d'entrée)
        fprintf(out, "%s;%.6f\n", idUsine, totalFuites);
        fclose(out);
        printf("Fuites totales : %.2f\n", totalFuites);
    }

    libererArbre(racine);
    freeRef(index);
    return 0;
}
