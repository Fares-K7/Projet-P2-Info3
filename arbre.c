#include "arbre.h"
#include <stdlib.h>
#include <string.h>

Noeud* creerNoeud(const char* id) {
    Noeud* n = malloc(sizeof(Noeud));
    if (!n) return NULL;
    strncpy(n->id, id, 99);
    n->id[99] = '\0';
    n->nbEnfants = 0;
    n->capEnfants = 2; // Capacité initiale faible
    n->enfants = malloc(2 * sizeof(Noeud*));
    n->taux = 0;
    return n;
}

void ajouterEnfant(Noeud* parent, Noeud* enfant) {
    if (!parent || !enfant) return;
    
    if (parent->nbEnfants == parent->capEnfants) {
        parent->capEnfants *= 2;
        parent->enfants = realloc(parent->enfants, parent->capEnfants * sizeof(Noeud*));
    }
    parent->enfants[parent->nbEnfants++] = enfant;
}

void libererArbre(Noeud* n) {
    if (!n) return;
    for (int i=0; i<n->nbEnfants; i++) {
        libererArbre(n->enfants[i]);
    }
    free(n->enfants);
    free(n);
}
