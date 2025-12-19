#ifndef AVL_H
#define AVL_H

#include <stdio.h>

// Structure du noeud AVL pour les usines
typedef struct NoeudAVL {
    char identifiant[100];
    double volumeMax;
    double volumeCapte;
    double volumeTraite;
    int equilibre; // Facteur d'équilibre
    struct NoeudAVL *gauche; // fg
    struct NoeudAVL *droit;  // fd
} NoeudAVL;

typedef struct {
    NoeudAVL* racine;
} AVL;

// Fonctions principales
AVL* creerAVL();
void libererAVL(AVL* avl);
int insererAVL(AVL* avl, const char* id);
NoeudAVL* rechercherUsine(AVL* avl, const char* id);

// Helpers pour remplir les données
void ajouterVolumeCapte(NoeudAVL* n, float v);
void ajouterVolumeTraite(NoeudAVL* n, float v);
void definirVolumeMax(NoeudAVL* n, float v);

// Entrées/Sorties
int ecrireAVLDansFichier(AVL* avl, FILE* f, const char* mode);
int compterNoeuds(AVL* avl);
void ecrireAVLDansFichier_Inverse(AVL* avl, FILE* f, const char* mode);
#endif
