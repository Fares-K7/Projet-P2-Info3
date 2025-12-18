#!/bin/bash

# ==========================================
# SCRIPT DE DIAGNOSTIC C-WIRE (LINUX)
# ==========================================

if [ "$#" -lt 3 ]; then
    echo "Usage: $0 <fichier.csv> <histo|leaks> <param>"
    exit 1
fi

CSV="$1"
CMD="$2"
OPT="$3"
EXE="./c-wire"

# 1. Compilation
echo "[1/4] Compilation..."
make
if [ $? -ne 0 ]; then echo "ERREUR CRITIQUE: Compilation échouée."; exit 1; fi

# 2. Nettoyage
rm -f output/*.dat tmp/*.dat tests/*.png
mkdir -p tmp tests output

# 3. Exécution C
echo "[2/4] Exécution du programme C..."
"$EXE" "$CSV" "$CMD" "$OPT"

# --- VÉRIFICATION CRITIQUE DES DONNÉES ---
DAT="output/vol_${OPT}.dat"

if [ ! -f "$DAT" ]; then
    echo "ERREUR CRITIQUE: Le fichier $DAT n'a pas été créé !"
    echo "Votre programme C n'a pas réussi à créer le fichier de sortie."
    exit 1
fi

if [ ! -s "$DAT" ]; then
    echo "ERREUR CRITIQUE: Le fichier $DAT est VIDE (0 octets) !"
    echo "Cela signifie que le programme C n'a trouvé aucune donnée à écrire."
    echo "Causes possibles :"
    echo "1. Le fichier CSV ($CSV) est vide ou introuvable."
    echo "2. Le format du CSV est mauvais (virgules au lieu de points-virgules ?)."
    echo "3. Le filtre (max/src/real) ne correspond à rien dans le CSV."
    exit 1
else
    echo "SUCCÈS: Données générées dans $DAT."
    # On affiche les 3 premières lignes pour vérifier
    echo "Aperçu des données :"
    head -n 3 "$DAT"
fi

# 4. Génération Gnuplot
if [ "$CMD" = "histo" ]; then
    echo "[3/4] Préparation des données pour Gnuplot..."
    # Tri
    tail -n +2 "$DAT" | sort -t';' -k2 -nr > tmp/sorted.dat
    head -n 10 tmp/sorted.dat >> tmp/top10.dat
    tail -n 50 tmp/sorted.dat | sort -t';' -k2 -n >> tmp/bot50.dat
    
    # Vérification que le tri a marché
    if [ ! -s "tmp/top10.dat" ]; then
        echo "ERREUR: Le fichier trié est vide. Problème avec la commande 'sort' ?"
        exit 1
    fi

    echo "[4/4] Dessin des graphiques..."
    
    # Top 10
    gnuplot -e "set terminal png size 1200,800; set output 'tests/vol_${OPT}_top10.png'; set title 'Top 10'; set style data histograms; set style fill solid; set datafile separator ';'; plot 'tmp/top10.dat' using 2:xtic(1) notitle"
    
    # Bottom 50
    gnuplot -e "set terminal png size 1200,800; set output 'tests/vol_${OPT}_bot50.png'; set title 'Bot 50'; set style data histograms; set style fill solid; set datafile separator ';'; plot 'tmp/bot50.dat' using 2:xtic(1) notitle"

    echo "TERMINE ! Vérifiez le dossier 'tests/'."
    ls -l tests/*.png
fi
