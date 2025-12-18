#ifndef ARBRE_H
#define ARBRE_H

typedef struct Noeud {
    char id[100];
    struct Noeud** enfants; // Tableau dynamique de pointeurs vers fils
    int nbEnfants;
    int capEnfants;
    float taux;
} Noeud;

Noeud* creerNoeud(const char* id);
void ajouterEnfant(Noeud* parent, Noeud* enfant);
void libererArbre(Noeud* racine);

#endif
