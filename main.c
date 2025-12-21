#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

    // --- MODE HISTOGRAMME ---
    if (strcmp(mode, "histo") == 0) {
        // Le sujet impose : max, src, ou real
        if (strcmp(param, "max") != 0 && 
            strcmp(param, "src") != 0 && 
            strcmp(param, "real") != 0) {
            
            printf("ERREUR: Pour 'histo', le paramètre doit être 'max', 'src' ou 'real'.\n");
            return 1;
        }
        return traiterHistogramme(fichier, param);
    } 
    
    // --- MODE FUITES (LEAKS) ---
    else if (strcmp(mode, "leaks") == 0) {
        return traiterFuites(fichier, param);
    }
    
    printf("Mode inconnu (utilisez 'histo' ou 'leaks').\n");
    return 1;
}
