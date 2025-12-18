#include "fichier.h"
#include <string.h>
#include <stdlib.h>

FILE* ouvrirFichierCSV(const char* chemin) {
    if (chemin == NULL) return stdin;
    FILE* f = fopen(chemin, "r");
    if (!f) perror("Erreur ouverture CSV");
    return f;
}

void fermerFichierCSV(FILE* f) {
    if (f && f != stdin) fclose(f);
}

int lireLigneCSV(FILE* f, LigneCSV* ligne) {
    char buffer[TAILLE_LIGNE];
    if (!fgets(buffer, TAILLE_LIGNE, f)) return 0;
    
    buffer[strcspn(buffer, "\r\n")] = 0;
    if (strlen(buffer) == 0) return 0;

    char* ptr = buffer;
    char* end;
    
    #define EXTRAIRE(dest) \
        end = strchr(ptr, ';'); \
        if (end) { *end = 0; strncpy(dest, ptr, TAILLE_CHAMP-1); ptr = end + 1; } \
        else { strncpy(dest, ptr, TAILLE_CHAMP-1); ptr += strlen(ptr); } \
        if (strlen(dest)==0) strcpy(dest, "-");

    EXTRAIRE(ligne->col1); EXTRAIRE(ligne->col2);
    EXTRAIRE(ligne->col3); EXTRAIRE(ligne->col4);
    EXTRAIRE(ligne->col5);
    return 1;
}

TypeLigneCSV identifierTypeLigne(const LigneCSV* l) {
    if (strcmp(l->col1, "-") == 0 && strstr(l->col3, "Facility")) return TYPE_SOURCE_USINE;
    if (strstr(l->col2, "Facility") && strcmp(l->col3, "-") == 0) return TYPE_USINE_NOEUD;
    if (strstr(l->col2, "Facility") && strstr(l->col3, "Storage")) return TYPE_USINE_STOCKAGE;
    if (strstr(l->col2, "Storage") && strstr(l->col3, "Junction")) return TYPE_STOCKAGE_JONCTION;
    if (strstr(l->col2, "Junction") && strstr(l->col3, "Service")) return TYPE_JONCTION_RACCORDEMENT;
    if (strstr(l->col2, "Service") && strstr(l->col3, "Cust")) return TYPE_RACCORDEMENT_USAGER;
    return TYPE_AUTRE;
}

int extraireIDUsine(const LigneCSV* l, char* buf) { strcpy(buf, l->col1); return strcmp(buf,"-")!=0; }
int extraireIDAmont(const LigneCSV* l, char* buf) { strcpy(buf, l->col2); return strcmp(buf,"-")!=0; }
int extraireIDAval(const LigneCSV* l, char* buf) { strcpy(buf, l->col3); return strcmp(buf,"-")!=0; }
int extraireVolume(const LigneCSV* l, float* v) { *v = atof(l->col4); return strcmp(l->col4,"-")!=0; }
int extraireTaux(const LigneCSV* l, float* t) { *t = atof(l->col5); return strcmp(l->col5,"-")!=0; }
