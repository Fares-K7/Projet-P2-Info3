#!/bin/bash

# =========================================================
# SCRIPT FINAL C-WIRE / WILDWATER
# =========================================================

# 1. Vérification des arguments
if [ "$#" -lt 3 ]; then
    echo "Usage: $0 <fichier.csv> <histo|leaks> <param>"
    echo "Exemple Histo: $0 c-wildwater_v3.dat histo max"
    echo "Exemple Leaks: $0 c-wildwater_v3.dat leaks \"Facility complex #...\""
    exit 1
fi

CSV="$1"
CMD="$2"
OPT="$3"
EXE="./c-wire"

# 2. Compilation
echo "[1/4] Compilation..."
make
if [ $? -ne 0 ]; then 
    echo "ERREUR CRITIQUE : Compilation échouée."
    exit 1 
fi

# 3. Nettoyage (On garde les output pour leaks)
rm -f tmp/*.dat tests/*.png

# Création des dossiers si inexistants
mkdir -p tmp tests output

# 4. Exécution du programme C
echo "[2/4] Exécution du programme C..."
"$EXE" "$CSV" "$CMD" "$OPT"

# ==========================================
# TRAITEMENT HISTOGRAMMES (HISTO)
# ==========================================
if [ "$CMD" = "histo" ]; then
    DAT="output/vol_${OPT}.dat"

    if [ ! -f "$DAT" ]; then
        echo "ERREUR : Fichier $DAT introuvable."
        exit 1
    fi

    echo "[3/4] Préparation Gnuplot..."
    
    # 1. Filtrage des zéros et Tri (Décroissant)
    tail -n +2 "$DAT" | grep -v ";0\.000" | sort -t';' -k2 -nr > tmp/sorted.dat

    NB_LIGNES=$(wc -l < tmp/sorted.dat)
    if [ "$NB_LIGNES" -eq 0 ]; then
        echo "ATTENTION : Aucune donnée non-nulle trouvée."
        exit 0
    fi

    # 2. Extraction : TOP 10 et BOTTOM 50
    head -n 10 tmp/sorted.dat > tmp/top10.dat
    tail -n 50 tmp/sorted.dat > tmp/bot50.dat  # <-- CHANGEMENT ICI (50 lignes)

    echo "[4/4] Génération des graphiques..."

    # Graphique MAX (Top 10) - Reste inchangé
    gnuplot -e "
        set terminal png size 1200,800; 
        set output 'tests/${OPT}_max.png'; 
        set title '${OPT} Max (Top 10)'; 
        set style data histograms; set style fill solid; set boxwidth 0.5; 
        set xtics rotate by -45 scale 0; set datafile separator ';'; set grid y;
        plot 'tmp/top10.dat' using 2:xtic(1) title '${OPT} Max' linecolor rgb '#228B22'
    "
    
    # Graphique MIN (Bottom 50) - Modifié pour 50 valeurs
    gnuplot -e "
        set terminal png size 1200,800; 
        set output 'tests/${OPT}_min.png'; 
        set title '${OPT} Min (Bottom 50)'; 
        set style data histograms; set style fill solid; set boxwidth 0.5; 
        set xtics rotate by -90 scale 0 font ',8'; set datafile separator ';'; set grid y;
        plot 'tmp/bot50.dat' using 2:xtic(1) title '${OPT} Min' linecolor rgb '#DC143C'
    "

    echo "TERMINE : tests/${OPT}_max.png (Top 10) et tests/${OPT}_min.png (Bottom 50) générés."
fi
# ==========================================
# TRAITEMENT FUITES (LEAKS)
# ==========================================
if [ "$CMD" = "leaks" ]; then
    echo "[3/4] Résultats des fuites pour : $OPT"
    
    # CORRECTION ICI : On pointe vers le nom fixe défini dans fuites.c
    LEAK_FILE="output/leaks.dat"

    if [ ! -f "$LEAK_FILE" ]; then
        echo "ERREUR : Le fichier '$LEAK_FILE' n'a pas été généré."
        exit 1
    fi

    echo ""
    echo "========================================================"
    # On utilise cat simple car column n'est peut-être pas installé ou bug avec les espaces
    cat "$LEAK_FILE"
    echo "========================================================"
    echo ""
fi
