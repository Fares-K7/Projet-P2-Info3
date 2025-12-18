#ifndef FICHIER_H
#define FICHIER_H

#include <stdio.h>

#define TAILLE_LIGNE 1024
#define TAILLE_CHAMP 100

typedef struct {
    char col1[TAILLE_CHAMP];
    char col2[TAILLE_CHAMP];
    char col3[TAILLE_CHAMP];
    char col4[TAILLE_CHAMP];
    char col5[TAILLE_CHAMP];
} LigneCSV;

typedef enum {
    TYPE_SOURCE_USINE, TYPE_USINE_NOEUD, TYPE_USINE_STOCKAGE,
    TYPE_STOCKAGE_JONCTION, TYPE_JONCTION_RACCORDEMENT,
    TYPE_RACCORDEMENT_USAGER, TYPE_AUTRE
} TypeLigneCSV;

FILE* ouvrirFichierCSV(const char* chemin);
void fermerFichierCSV(FILE* f);
int lireLigneCSV(FILE* f, LigneCSV* ligne);
TypeLigneCSV identifierTypeLigne(const LigneCSV* ligne);

int extraireIDUsine(const LigneCSV* l, char* buf);
int extraireIDAmont(const LigneCSV* l, char* buf);
int extraireIDAval(const LigneCSV* l, char* buf);
int extraireVolume(const LigneCSV* l, float* v);
int extraireTaux(const LigneCSV* l, float* t);

#endif
