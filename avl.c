#include "avl.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

// --- Fonctions Internes (Static) ---

static NoeudAVL* creerNoeud(const char* identifiant) {
    NoeudAVL* n = malloc(sizeof(NoeudAVL));
    if (!n) return NULL;
    strncpy(n->identifiant, identifiant, 99);
    n->identifiant[99] = '\0';
    n->volumeMax = 0;
    n->volumeCapte = 0;
    n->volumeTraite = 0;
    n->equilibre = 0;
    n->gauche = NULL;
    n->droit = NULL;
    return n;
}

static int hauteur(NoeudAVL* n) {
    if (!n) return 0;
    return 1 + max(hauteur(n->gauche), hauteur(n->droit));
}

// Calcul du facteur d'équilibre (H_Droit - H_Gauche)
static int facteurEquilibre(NoeudAVL* n) {
    if (!n) return 0;
    return hauteur(n->droit) - hauteur(n->gauche);
}

// Rotation Gauche
static NoeudAVL* rotationGauche(NoeudAVL* a) {
    if (!a || !a->droit) return a;
    NoeudAVL* b = a->droit;
    a->droit = b->gauche;
    b->gauche = a;
    return b;
}

// Rotation Droite
static NoeudAVL* rotationDroite(NoeudAVL* a) {
    if (!a || !a->gauche) return a;
    NoeudAVL* b = a->gauche;
    a->gauche = b->droit;
    b->droit = a;
    return b;
}

// Double Rotation Gauche
static NoeudAVL* doubleRotationGauche(NoeudAVL* a) {
    a->droit = rotationDroite(a->droit);
    return rotationGauche(a);
}

// Double Rotation Droite
static NoeudAVL* doubleRotationDroite(NoeudAVL* a) {
    a->gauche = rotationGauche(a->gauche);
    return rotationDroite(a);
}

// Algorithme d'équilibrage du cours (Page 196)
static NoeudAVL* equilibrer(NoeudAVL* a) {
    if (!a) return NULL;
    
    int eq = facteurEquilibre(a);

    // Cas Droite Lourde (>= 2)
    if (eq >= 2) {
        if (facteurEquilibre(a->droit) >= 0) {
            return rotationGauche(a);
        } else {
            return doubleRotationGauche(a);
        }
    }
    // Cas Gauche Lourde (<= -2)
    else if (eq <= -2) {
        if (facteurEquilibre(a->gauche) <= 0) {
            return rotationDroite(a);
        } else {
            return doubleRotationDroite(a);
        }
    }
    return a;
}

static NoeudAVL* insert_rec(NoeudAVL* n, const char* id, int* succes) {
    if (!n) { 
        *succes = 1; 
        return creerNoeud(id); 
    }
    
    int cmp = strcmp(id, n->identifiant);
    
    if (cmp < 0) {
        n->gauche = insert_rec(n->gauche, id, succes);
    } else if (cmp > 0) {
        n->droit = insert_rec(n->droit, id, succes);
    } else {
        *succes = 1; // Déjà présent
        return n;
    }
    
    return equilibrer(n);
}

// --- Fonctions Publiques ---

AVL* creerAVL() {
    AVL* avl = malloc(sizeof(AVL));
    if (avl) avl->racine = NULL;
    return avl;
}

int insererAVL(AVL* avl, const char* id) {
    if (!avl) return 0;
    int succes = 0;
    avl->racine = insert_rec(avl->racine, id, &succes);
    return succes;
}

NoeudAVL* rechercherUsine(AVL* avl, const char* id) {
    if (!avl) return NULL;
    NoeudAVL* n = avl->racine;
    while (n) {
        int cmp = strcmp(id, n->identifiant);
        if (cmp == 0) return n;
        if (cmp < 0) n = n->gauche;
        else n = n->droit;
    }
    return NULL;
}

void ajouterVolumeCapte(NoeudAVL* n, float v) { if (n) n->volumeCapte += v; }
void ajouterVolumeTraite(NoeudAVL* n, float v) { if (n) n->volumeTraite += v; }
void definirVolumeMax(NoeudAVL* n, float v) { if (n) n->volumeMax = v; }

static void parcours(NoeudAVL* n, FILE* f, const char* mode) {
    if (!n) return;
    parcours(n->gauche, f, mode);
    
    if (strcmp(mode, "max")==0) fprintf(f, "%s;%.3f\n", n->identifiant, n->volumeMax);
    else if (strcmp(mode, "src")==0) fprintf(f, "%s;%.3f\n", n->identifiant, n->volumeCapte);
    else fprintf(f, "%s;%.3f\n", n->identifiant, n->volumeTraite);
    
    parcours(n->droit, f, mode);
}

int ecrireAVLDansFichier(AVL* avl, FILE* f, const char* mode) {
    if (!avl || !f) return 0;
    parcours(avl->racine, f, mode);
    return 1;
}

static void liberer_rec(NoeudAVL* n) {
    if (!n) return;
    liberer_rec(n->gauche);
    liberer_rec(n->droit);
    free(n);
}

void libererAVL(AVL* avl) {
    if (avl) { liberer_rec(avl->racine); free(avl); }
}

static int compter_rec(NoeudAVL* n) {
    if (!n) return 0;
    return 1 + compter_rec(n->gauche) + compter_rec(n->droit);
}

int compterNoeuds(AVL* avl) {
    if (!avl) return 0;
    return compter_rec(avl->racine);
}
