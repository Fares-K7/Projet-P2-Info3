#include "utils.h"
#include <sys/stat.h>
#include <sys/types.h>

void creerRepertoire(const char* chemin) {
    struct stat st = {0};
    if (stat(chemin, &st) == -1) {
        #ifdef _WIN32
            mkdir(chemin);
        #else
            mkdir(chemin, 0700);
        #endif
    }
}
