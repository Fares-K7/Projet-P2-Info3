#!/bin/bash
# myScript.sh - Script d'orchestration pour Projet C-WildWater

# ----------------- CONFIGURATION -----------------
C_EXEC="./wildwater_app"
CSV_FILE="data.csv" # Le fichier de données fourni
HISTORY_FILE="leaks_history.dat"
# -------------------------------------------------

# --- 1. Démarrer le chronomètre et définir les fonctions de sortie ---
START_TIME=$(date +%s)

function exit_script() {
    local EXIT_CODE=$1
    local END_TIME=$(date +%s)
    local DURATION=$((END_TIME - START_TIME))
    echo "Durée totale du script : $DURATION secondes"
    exit $EXIT_CODE
}

function usage_error() {
    echo "Erreur : Commande incorrecte ou incomplète." >&2
    echo "Usage : $0 histo (max|src|real)" >&2
    echo "Usage : $0 leaks \"ID_USINE\"" >&2
    exit_script 1
}

# --- 2. Vérification des arguments ---
if [ $# -lt 2 ] || [ $# -gt 2 ]; then
    usage_error
fi

COMMAND=$1
ARGUMENT=$2

if [ "$COMMAND" != "histo" ] && [ "$COMMAND" != "leaks" ]; then
    usage_error
fi

if [ "$COMMAND" == "histo" ] && [ "$ARGUMENT" != "max" ] && [ "$ARGUMENT" != "src" ] && [ "$ARGUMENT" != "real" ]; then
    usage_error
fi

# --- 3. Compilation du programme C ---
if [ ! -f "$C_EXEC" ]; then
    echo "L'exécutable C n'existe pas. Tentative de compilation..."
    make
    if [ $? -ne 0 ]; then
        echo "Erreur (make) : La compilation du programme C a échoué. (Code $?)" >&2
        exit_script 2
    fi
fi
echo "Compilation réussie. Exécution du traitement."

# --- 4. Exécution du C et Traitement Histogramme ---
if [ "$COMMAND" == "histo" ]; then
    OUTPUT_FILE="${ARGUMENT}_volume.dat"
    IMAGE_FILE="${ARGUMENT}_volume.png"

    echo "Appel C pour l'histogramme... (résultat dans $OUTPUT_FILE)"
    # Le C génère le fichier .dat trié par ID inverse
    $C_EXEC histo "$ARGUMENT" "$CSV_FILE" "$OUTPUT_FILE"
    C_RETURN_CODE=$?

    if [ $C_RETURN_CODE -ne 0 ]; then
        echo "Erreur ($C_RETURN_CODE) lors du traitement C 'histo'." >&2
        exit_script 3
    fi

    # --- 5. Génération de l'histogramme (GNUplot) ---
    echo "Génération de l'image de l'histogramme..."

    # Créer un fichier de données temporaire pour les 10 entrées (les 5 min et 5 max)
    TEMP_DATA_FILE="temp_plot_data.dat"

    # L'entête doit être conservée
    head -n 1 "$OUTPUT_FILE" > "$TEMP_DATA_FILE"

    # 1. Obtenir les 5 plus petites (par volume, colonne 2, séparateur ';')
    # Utiliser tail pour ignorer l'entête
    tail -n +2 "$OUTPUT_FILE" | sort -t ';' -n -k2,2 | head -n 5 >> "$TEMP_DATA_FILE"

    # 2. Obtenir les 5 plus grandes (tri inverse par volume)
    tail -n +2 "$OUTPUT_FILE" | sort -t ';' -n -r -k2,2 | head -n 5 >> "$TEMP_DATA_FILE"

    # Définition des titres pour Gnuplot
    case "$ARGUMENT" in
        max)
            Y_TITLE="Capacité Maximale (k.m³.year⁻¹)";;
        src)
            Y_TITLE="Volume Capté Total (k.m³.year⁻¹)";;
        real)
            Y_TITLE="Volume Traité Réel (k.m³.year⁻¹)";;
    esac

    # Création du script Gnuplot
    GNUPLOT_SCRIPT="histo_${ARGUMENT}.gp"
    cat > "$GNUPLOT_SCRIPT" << EOF
set terminal png size 1200,800
set output "$IMAGE_FILE"
set datafile separator ";"
set style data histogram
set style histogram cluster gap 1
set style fill solid border -1
set boxwidth 0.9
set xtics rotate by -45 offset 0, -1
set title "Top 5 et Bottom 5 des Usines par Volume (${ARGUMENT})"
set ylabel "$Y_TITLE"
set xlabel "Identifiant Usine"
set key off
plot "$TEMP_DATA_FILE" using 2:xticlabels(1) title "$Y_TITLE"
EOF

    # Exécution de Gnuplot
    gnuplot "$GNUPLOT_SCRIPT"
    if [ $? -ne 0 ]; then
        echo "Avertissement : Gnuplot n'a pas pu générer l'image. (Code $?)." >&2
    fi

    # Nettoyage des fichiers temporaires
    rm "$GNUPLOT_SCRIPT" "$TEMP_DATA_FILE"

# --- 6. Exécution du C et Traitement Fuites ---
elif [ "$COMMAND" == "leaks" ]; then

    echo "Appel C pour le calcul des fuites pour l'usine '$ARGUMENT'..."
    # Le C calcule les fuites et écrit dans le fichier historique
    $C_EXEC leaks "$ARGUMENT" "$CSV_FILE" "$HISTORY_FILE"
    C_RETURN_CODE=$?

    if [ $C_RETURN_CODE -ne 0 ]; then
        if [ $C_RETURN_CODE -eq 2 ]; then
            echo "Erreur (ID non trouvé) : L'identifiant d'usine '$ARGUMENT' est inconnu ou non-traitant." >&2
        else
            echo "Erreur ($C_RETURN_CODE) lors du traitement C 'leaks'." >&2
        fi
        exit_script 4
    fi

fi

# --- 7. Fin du script ---
exit_script 0