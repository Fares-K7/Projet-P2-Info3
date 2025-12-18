#include <stdio.h>
#include <string.h>
#include "usine.h"
#include "fuites.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s <fichier.csv> <histo|leaks> <param>\n", argv[0]);
        return 1;
    }

    char* fichier = argv[1];
    char* mode = argv[2];
    char* param = argv[3];

    creerRepertoire("tests");
    creerRepertoire("output");
    creerRepertoire("tmp");

    if (strcmp(mode, "histo") == 0) {
        return traiterHistogramme(fichier, param);
    } 
    else if (strcmp(mode, "leaks") == 0) {
        return traiterFuites(fichier, param);
    }
    
    return 1;
}
