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

    # Vérification simple et robuste
    if [ ! -f "$DAT" ]; then
        echo "ERREUR : Le fichier $DAT n'existe pas. Vérifie le 'make'."
        exit 1
    fi

    echo "[3/4] Préparation Gnuplot..."

    # 1. On trie les données (décroissant) et on filtre les 0
    tail -n +2 "$DAT" | grep -v ";0\.000" | sort -t';' -k2 -nr > tmp/sorted.dat

    # Vérification qu'il y a des données
    if [ ! -s tmp/sorted.dat ]; then
        echo "Attention : Aucune donnée à afficher (fichier vide ou que des zéros)."
        exit 0
    fi

    # 2. On extrait le TOP 10 (Max) et les 50 plus petits (Min)
    head -n 10 tmp/sorted.dat > tmp/top10.dat
    tail -n 50 tmp/sorted.dat > tmp/50derniers.dat

    echo "[4/4] Génération des graphiques avec légendes..."

    # --- Graphique MAX ---
    gnuplot -e "
        set terminal png size 1200,800; 
        set output 'tests/${OPT}_max.png'; 
        set title 'volume-${OPT} (Top 10)'; 
        set xlabel 'Identifiant Station';     
        set ylabel 'Volume (m3)';             
        set style data histograms; set style fill solid; set boxwidth 0.5; 
        set xtics rotate by -45 scale 0; set datafile separator ';'; set grid y;
        plot 'tmp/top10.dat' using 2:xtic(1) title 'Max' linecolor rgb '#228B22'
    "
    
    # --- Graphique MIN ---
    gnuplot -e "
        set terminal png size 1200,800; 
        set output 'tests/${OPT}_min.png'; 
        set title 'volume-${OPT} (50 Derniers)'; 
        set xlabel 'Identifiant Station';     
        set ylabel 'Volume (m3)';             
        set style data histograms; set style fill solid; set boxwidth 0.5; 
        set xtics rotate by -90 scale 0 font ',8'; set datafile separator ';'; set grid y;
        plot 'tmp/50derniers.dat' using 2:xtic(1) title 'Min' linecolor rgb '#DC143C'
    "

    echo "Terminé : Graphiques générés dans le dossier tests/."
fi
# ==========================================
# TRAITEMENT FUITES (LEAKS)
# ==========================================
if [ "$CMD" = "leaks" ]; then
    echo "[3/4] Résultats des fuites pour : $OPT"
    
    LEAK_FILE="output/leaks.dat"

    if [ ! -f "$LEAK_FILE" ]; then
        echo "ERREUR : Le fichier '$LEAK_FILE' n'a pas été généré."
        exit 1
    fi

    echo ""
    echo "========================================================"
    cat "$LEAK_FILE"
    echo "========================================================"
    echo ""
fi
