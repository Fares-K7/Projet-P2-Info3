#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define COL_FACTORY_ID 1
#define COL_AMONT_ID   2
#define COL_AVAL_ID    3
#define COL_VOLUME     4
#define COL_LEAK_PCT   5
#define MAX_LINE_LENGTH 1024

typedef struct {
    char *id;           // Identifiant unique de l'usine (clé de tri pour l'AVL)
    double max_capacity; // Capacité maximale de traitement (k.m³) [cite: 86, 145]
    double total_captured; // Volume total capté par les sources (k.m³) [cite: 147]
    double total_treated;  // Volume total réellement traité (k.m³) [cite: 150]
} FactoryData;

// 2. Nœud de l'AVL (pour un accès rapide aux usines) [cite: 203]
typedef struct AVLFactoryNode {
    FactoryData data;
    int height;
    struct AVLFactoryNode *left;
    struct AVLFactoryNode *right;
} AVLFactoryNode;

// --- Prototypes des fonctions de l'AVL ---

/**
 * @brief Crée et initialise un nouveau nœud AVL d'usine.
 * @param id L'identifiant de l'usine.
 * @return Un pointeur vers le nouveau nœud.
 */
AVLFactoryNode *create_factory_node(const char *id);

/**
 * @brief Insère un nouveau nœud d'usine dans l'AVL ou met à jour un existant.
 * @param root Pointeur vers la racine de l'AVL.
 * @param id L'identifiant de l'usine.
 * @return La nouvelle racine de l'AVL.
 */
AVLFactoryNode *insert_factory(AVLFactoryNode *root, const char *id, double capacity);

/**
 * @brief Recherche une usine par son identifiant.
 * @param root Pointeur vers la racine de l'AVL.
 * @param id L'identifiant à chercher.
 * @return Pointeur vers le nœud s'il est trouvé, NULL sinon.
 */
AVLFactoryNode *search_factory(AVLFactoryNode *root, const char *id);

/**
 * @brief Met à jour les volumes captés et traités pour une usine.
 * @param factory_node Le nœud de l'usine.
 * @param captured_volume Volume capté par une source.
 * @param leak_percentage Pourcentage de fuite entre source et usine.
 */
void update_factory_volumes(AVLFactoryNode *factory_node, double captured_volume, float leak_percentage);

/**
 * @brief Génère le fichier de données CSV pour l'histogramme (trié par ID inverse). [cite: 154, 158]
 * @param root Pointeur vers la racine de l'AVL.
 * @param criteria Le critère de volume à utiliser ('max', 'src', 'real').
 * @param filepath Chemin du fichier de sortie.
 * @return 0 en cas de succès, 1 sinon.
 */
int generate_histogram_data(AVLFactoryNode *root, const char *criteria, const char *filepath);

/**
 * @brief Libère toute la mémoire de l'AVL des usines. [cite: 218]
 * @param root Pointeur vers la racine.
 */
void free_factory_avl(AVLFactoryNode *root);

// 1. Représentation du tronçon (lien Parent -> Enfant)
typedef struct ChildLink {
    char *aval_id;                  // Identifiant de l'acteur aval
    float leak_percentage;          // Pourcentage de fuite sur ce tronçon [cite: 64, 95]
    struct DistributionNode *child_node; // Pointeur vers le nœud enfant
    struct ChildLink *next;
} ChildLink;

// 2. Nœud du graphe (acteur : Usine, Stockage, Jonction, Raccordement)
typedef struct DistributionNode {
    char *id;           // Identifiant de l'acteur (clé de l'AVL de recherche)
    char *factory_id;   // Usine qui a traité l'eau [cite: 66, 106]
    ChildLink *children; // Liste chaînée des acteurs aval (enfants) [cite: 205]
    // Autres champs utiles (ex: type de nœud pour debug)
} DistributionNode;

// 3. Nœud de l'AVL pour la recherche d'adresse des nœuds (accélère la construction du graphe) [cite: 207]
typedef struct AVLSearchNode {
    char *id; // Identifiant de l'acteur (clé AVL)
    DistributionNode *address; // Adresse du nœud dans le graphe
    int height;
    struct AVLSearchNode *left;
    struct AVLSearchNode *right;
} AVLSearchNode;

// --- Prototypes des fonctions de la fuite et du graphe ---

/**
 * @brief Construit le graphe de distribution en utilisant l'AVL de recherche.
 * @param root_search Pointeur vers la racine de l'AVL de recherche (passé par référence).
 * @param factory_root Pointeur vers la racine de l'AVL des usines.
 * @param filepath Chemin du fichier CSV.
 * @return Pointeur vers le nœud racine de l'AVL de recherche, NULL si erreur.
 */
AVLSearchNode *build_distribution_graph(AVLSearchNode *root_search, AVLFactoryNode *factory_root, const char *filepath);

/**
 * @brief Calcule récursivement le volume d'eau perdu en aval d'un nœud.
 * @param node Le nœud de distribution actuel.
 * @param volume_in Le volume d'eau entrant dans ce nœud (avant répartition).
 * @param total_leak_volume Pointeur vers le cumul des pertes.
 */
void recursive_leak_calculation(DistributionNode *node, double volume_in, double *total_leak_volume);

/**
 * @brief Fonction principale pour calculer les fuites d'une usine.
 * @param csv_filepath Chemin du fichier CSV.
 * @param factory_id Identifiant de l'usine cible.
 * @param history_filepath Chemin du fichier d'historique.
 * @return 0 en cas de succès, code d'erreur > 0 sinon.
 */
int calculate_leaks(const char *csv_filepath, const char *factory_id, const char *history_filepath);

/**
 * @brief Libère la mémoire du graphe de distribution. [cite: 218]
 * @param root_search Pointeur vers la racine de l'AVL de recherche.
 */
void free_distribution_graph(AVLSearchNode *root_search);

AVLSearchNode *insert_search_node(AVLSearchNode *node, const char *id, DistributionNode *address);

typedef enum {
    TYPE_SOURCE_TO_FACTORY,
    TYPE_FACTORY_NODE,
    TYPE_FACTORY_TO_STORAGE,
    TYPE_STORAGE_TO_JUNCTION,
    TYPE_JUNCTION_TO_SERVICE,
    TYPE_SERVICE_TO_USER,
    TYPE_UNKNOWN
} LineType;

// --- Prototypes des fonctions d'analyse ---

/**
 * @brief Analyse une ligne du CSV et détermine son type.
 * @param line La ligne de texte à analyser.
 * @param cols Tableau pour stocker les 5 colonnes (les jetons).
 * @return Le type de la ligne.
 */
LineType parse_csv_line(char *line, char **cols);

/**
 * @brief Fonction pour lire et traiter toutes les données CSV pour l'histogramme.
 * @param filepath Chemin du fichier CSV.
 * @param factory_root Pointeur vers la racine de l'AVL des usines (passé par référence).
 * @return 0 en cas de succès, code d'erreur > 0 sinon.
 */
int parse_data_for_histogram(const char *filepath, AVLFactoryNode **factory_root);

/**
 * @brief Fonction pour lire et traiter toutes les données CSV pour le graphe de fuites.
 * Nécessite de faire DEUX passes : une pour créer tous les nœuds, une pour créer tous les liens.
 * @param filepath Chemin du fichier CSV.
 * @param factory_root Pointeur vers la racine de l'AVL des usines.
 * @param search_root Pointeur vers la racine de l'AVL de recherche (passé par référence).
 * @return 0 en cas de succès, code d'erreur > 0 sinon.
 */
int parse_data_for_graph(const char *filepath, AVLFactoryNode *factory_root, AVLSearchNode **search_root);



int process_histogram(const char *csv_filepath, const char *criteria, const char *output_filepath) {
    AVLFactoryNode *factory_root = NULL;

    // 1. Lire et remplir l'AVL des usines (besoin de parcourir le CSV)
    int result = parse_data_for_histogram(csv_filepath, &factory_root);
    if (result != 0) {
        free_factory_avl(factory_root);
        return result;
    }

    // 2. Générer le fichier de données CSV trié
    result = generate_histogram_data(factory_root, criteria, output_filepath);

    // 3. Libérer la mémoire
    free_factory_avl(factory_root);
    return result;
}



/** @brief Retourne la hauteur d'un nœud (0 si NULL). */
static int get_height(AVLFactoryNode *node) {
    if (node == NULL) return 0;
    return node->height;
}

/** @brief Calcule le facteur de balance d'un nœud. */
static int get_balance_factor(AVLFactoryNode *node) {
    if (node == NULL) return 0;
    return get_height(node->left) - get_height(node->right);
}

/** @brief Met à jour la hauteur d'un nœud. */
static void update_height(AVLFactoryNode *node) {
    if (node != NULL) {
        int left_h = get_height(node->left);
        int right_h = get_height(node->right);
        node->height = (left_h > right_h ? left_h : right_h) + 1;
    }
}

/** @brief Rotation droite pour rééquilibrer l'AVL. */
static AVLFactoryNode *right_rotate(AVLFactoryNode *y) {
    AVLFactoryNode *x = y->left;
    AVLFactoryNode *T2 = x->right;

    // Effectuer la rotation
    x->right = y;
    y->left = T2;

    // Mise à jour des hauteurs
    update_height(y);
    update_height(x);

    return x;
}

/** @brief Rotation gauche pour rééquilibrer l'AVL. */
static AVLFactoryNode *left_rotate(AVLFactoryNode *x) {
    AVLFactoryNode *y = x->right;
    AVLFactoryNode *T2 = y->left;

    // Effectuer la rotation
    y->left = x;
    x->right = T2;

    // Mise à jour des hauteurs
    update_height(x);
    update_height(y);

    return y;
}

// --- Implémentation des fonctions publiques ---

AVLFactoryNode *create_factory_node(const char *id) {
    AVLFactoryNode *node = (AVLFactoryNode *)malloc(sizeof(AVLFactoryNode));
    if (node == NULL) {
        perror("Erreur d'allocation pour AVLFactoryNode");
        return NULL;
    }
    node->data.id = strdup(id);
    if (node->data.id == NULL) {
        free(node);
        perror("Erreur d'allocation pour ID Usine");
        return NULL;
    }
    // Initialisation des données
    node->data.max_capacity = 0.0;
    node->data.total_captured = 0.0;
    node->data.total_treated = 0.0;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

AVLFactoryNode *insert_factory(AVLFactoryNode *node, const char *id, double capacity) {
    if (node == NULL) {
        // Nouvelle usine, création du nœud
        return create_factory_node(id);
    }

    int compare_result = strcmp(id, node->data.id);

    if (compare_result < 0) {
        node->left = insert_factory(node->left, id, capacity);
    } else if (compare_result > 0) {
        node->right = insert_factory(node->right, id, capacity);
    } else {
        // Usine déjà existante, mettre à jour la capacité (si applicable)
        if (capacity > 0.0) {
             node->data.max_capacity = capacity;
        }
        return node; // Ne rien faire ou retourner
    }

    // Mise à jour de la hauteur et rééquilibrage
    update_height(node);
    int balance = get_balance_factor(node);

    // Cas de rotations (4 cas standards de l'AVL)
    // Gauche-Gauche (LL)
    if (balance > 1 && strcmp(id, node->left->data.id) < 0)
        return right_rotate(node);

    // Droite-Droite (RR)
    if (balance < -1 && strcmp(id, node->right->data.id) > 0)
        return left_rotate(node);

    // Gauche-Droite (LR)
    if (balance > 1 && strcmp(id, node->left->data.id) > 0) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    // Droite-Gauche (RL)
    if (balance < -1 && strcmp(id, node->right->data.id) < 0) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

AVLFactoryNode *search_factory(AVLFactoryNode *root, const char *id) {
    if (root == NULL) return NULL;

    int compare_result = strcmp(id, root->data.id);

    if (compare_result == 0) {
        return root;
    } else if (compare_result < 0) {
        return search_factory(root->left, id);
    } else {
        return search_factory(root->right, id);
    }
}

void update_factory_volumes(AVLFactoryNode *factory_node, double captured_volume, float leak_percentage) {
    if (factory_node == NULL) return;

    // 1 k.m³ = 1000 m³
    double treated_volume = captured_volume * (1.0 - leak_percentage / 100.0);

    factory_node->data.total_captured += captured_volume;
    factory_node->data.total_treated += treated_volume;
}

// Structure temporaire pour le tri (pas dans le .h car interne)
typedef struct {
    char *id;
    double volume;
} FactorySortItem;

/** @brief Fonction de comparaison pour qsort (tri alphabétique inverse par ID). */
static int compare_factory_items(const void *a, const void *b) {
    const FactorySortItem *itemA = (const FactorySortItem *)a;
    const FactorySortItem *itemB = (const FactorySortItem *)b;
    // Tri alphabétique inverse (B vient avant A)
    return strcmp(itemB->id, itemA->id);
}

/** @brief Parcours de l'AVL pour collecter les données dans un tableau. */
static void collect_factory_data(AVLFactoryNode *root, const char *criteria, FactorySortItem **array, int *index, int *max_size) {
    if (root == NULL) return;

    // S'assurer que le tableau est assez grand
    if (*index >= *max_size) {
        *max_size *= 2;
        *array = (FactorySortItem *)realloc(*array, (*max_size) * sizeof(FactorySortItem));
        if (*array == NULL) {
            perror("Erreur realloc pour FactorySortItem");
            return;
        }
    }

    // Remplir la structure temporaire avec le volume basé sur les critères
    (*array)[*index].id = root->data.id; // Utiliser le pointeur existant (ne pas copier)
    if (strcmp(criteria, "max") == 0) {
        (*array)[*index].volume = root->data.max_capacity;
    } else if (strcmp(criteria, "src") == 0) {
        (*array)[*index].volume = root->data.total_captured;
    } else if (strcmp(criteria, "real") == 0) {
        (*array)[*index].volume = root->data.total_treated;
    } else {
        // Gérer le cas inconnu (ou ignorer)
        (*array)[*index].volume = 0.0;
    }

    (*index)++;

    collect_factory_data(root->left, criteria, array, index, max_size);
    collect_factory_data(root->right, criteria, array, index, max_size);
}

int generate_histogram_data(AVLFactoryNode *root, const char *criteria, const char *filepath) {
    if (root == NULL) return 1;

    // 1. Collecter les données de l'AVL dans un tableau
    int count = 0;
    int max_size = 50; // Taille initiale
    FactorySortItem *items = (FactorySortItem *)malloc(max_size * sizeof(FactorySortItem));
    if (items == NULL) {
        perror("Erreur d'allocation pour items");
        return 1;
    }

    collect_factory_data(root, criteria, &items, &count, &max_size);

    // 2. Retailler le tableau à la taille réelle
    if (count > 0) {
        items = (FactorySortItem *)realloc(items, count * sizeof(FactorySortItem));
    } else {
        free(items);
        return 1; // Aucun élément à traiter
    }

    // 3. Trier par ID d'usine alphabétique inverse (requis par le sujet)
    qsort(items, count, sizeof(FactorySortItem), compare_factory_items);

    // 4. Écrire dans le fichier de sortie
    FILE *f = fopen(filepath, "w");
    if (f == NULL) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        free(items);
        return 1;
    }

    // Écriture de l'entête
    fprintf(f, "Identifiant;Volume\n");

    for (int i = 0; i < count; i++) {
        fprintf(f, "%s;%lf\n", items[i].id, items[i].volume);
    }

    fclose(f);
    free(items); // Libérer le tableau temporaire
    return 0;
}

void free_factory_avl(AVLFactoryNode *root) {
    if (root == NULL) return;

    free_factory_avl(root->left);
    free_factory_avl(root->right);

    // Libérer les données allouées dynamiquement
    free(root->data.id);
    free(root);
}


static int get_search_height(AVLSearchNode *node) {
    if (node == NULL) return 0;
    return node->height;
}

static void update_search_height(AVLSearchNode *node) {
    if (node != NULL) {
        int left_h = get_search_height(node->left);
        int right_h = get_search_height(node->right);
        node->height = (left_h > right_h ? left_h : right_h) + 1;
    }
}

// (Implémenter ici les fonctions left_rotate et right_rotate pour AVLSearchNode)
// ... (omises ici pour la concision, mais nécessaires)

AVLSearchNode *create_search_node(const char *id, DistributionNode *address) {
    AVLSearchNode *node = (AVLSearchNode *)malloc(sizeof(AVLSearchNode));
    if (node == NULL) { /* Handle error */ return NULL; }

    node->id = strdup(id);
    if (node->id == NULL) { /* Handle error */ free(node); return NULL; }

    node->address = address;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}


/** @brief Rotation droite pour AVLSearchNode. */

/** @brief Rotation gauche pour AVLSearchNode. */


// ... (create_search_node est OK)


AVLSearchNode *search_distribution_node(AVLSearchNode *root, const char *id) {
    if (root == NULL) return NULL;
    int cmp = strcmp(id, root->id);
    if (cmp == 0) return root;
    if (cmp < 0) return search_distribution_node(root->left, id);
    return search_distribution_node(root->right, id);
}

// --- Fonctions de gestion du Graphe ---

DistributionNode *create_distribution_node(const char *id, const char *factory_id) {
    DistributionNode *node = (DistributionNode *)malloc(sizeof(DistributionNode));
    if (node == NULL) { /* Handle error */ return NULL; }

    node->id = strdup(id);
    node->factory_id = strdup(factory_id);
    node->children = NULL;

    // Si la duplication échoue, gérer l'erreur et libérer la mémoire.
    if (node->id == NULL || node->factory_id == NULL) {
        free(node->id);
        free(node->factory_id);
        free(node);
        return NULL;
    }

    return node;
}

void add_child_link(DistributionNode *parent, DistributionNode *child, float leak_pct) {
    ChildLink *new_link = (ChildLink *)malloc(sizeof(ChildLink));
    if (new_link == NULL) { /* Handle error */ return; }

    new_link->aval_id = strdup(child->id); // Dupliquer l'ID aval pour la construction
    new_link->child_node = child;
    new_link->leak_percentage = leak_pct;

    // Insertion en tête de la liste
    new_link->next = parent->children;
    parent->children = new_link;
}

// --- Fonction de calcul récursif ---

void recursive_leak_calculation(DistributionNode *node, double volume_in, double *total_leak_volume) {
    if (node == NULL) return;

    int num_children = 0;
    for (ChildLink *link = node->children; link != NULL; link = link->next) {
        num_children++;
    }

    if (num_children == 0) {
        // Terminus (Usager ou fin de réseau)
        return;
    }

    // Le volume entrant est réparti équitablement entre les enfants
    double volume_per_child = volume_in / num_children;

    // Parcourir chaque tronçon vers l'aval
    for (ChildLink *link = node->children; link != NULL; link = link->next) {
        float leak_pct = link->leak_percentage;

        // Calcul de la fuite sur ce tronçon
        double leak_on_segment = volume_per_child * (leak_pct / 100.0);

        // Ajout au total
        *total_leak_volume += leak_on_segment;

        // Volume transmis à l'enfant
        double volume_out = volume_per_child - leak_on_segment;

        // Appel récursif pour l'enfant
        if (link->child_node != NULL) {
            recursive_leak_calculation(link->child_node, volume_out, total_leak_volume);
        } else {
             // Cas où le nœud n'a pas été trouvé ou est un usager non représenté
             // L'eau est considérée comme perdue ou consommée à la fin du réseau connu
        }
    }
}

// --- Fonction principale leaks ---

int calculate_leaks(const char *csv_filepath, const char *factory_id, const char *history_filepath) {
    AVLFactoryNode *factory_root = NULL;
    AVLSearchNode *search_root = NULL;
    int result = 0;
    double total_leaks = 0.0;

    // --- 1. Lecture pour l'AVL des usines et des volumes amont ---
    if (parse_data_for_histogram(csv_filepath, &factory_root) != 0) {
        fprintf(stderr, "Erreur de lecture des données amont.\n");
        return 1;
    }

    // Trouver l'usine cible et son volume de départ
    AVLFactoryNode *target_factory_node = search_factory(factory_root, factory_id);
    if (target_factory_node == NULL) {
        fprintf(stderr, "Erreur : Usine ID '%s' introuvable.\n", factory_id);
        result = 2; // Erreur spécifique : ID inconnu
        goto cleanup;
    }

    double initial_volume = target_factory_node->data.total_treated; // Volume traité réel

    // --- 2. Construction du graphe de distribution ---
    if (parse_data_for_graph(csv_filepath, factory_root, &search_root) != 0) {
        fprintf(stderr, "Erreur de construction du graphe de distribution.\n");
        result = 3;
        goto cleanup;
    }

    // --- 3. Démarrer le calcul récursif ---
    // Le nœud de l'usine dans le GRAPHE (et non dans l'AVL des usines)
    AVLSearchNode *factory_in_graph = search_distribution_node(search_root, factory_id);

    if (factory_in_graph == NULL) {
        fprintf(stderr, "Erreur : L'usine n'est pas la racine d'un sous-réseau aval.\n");
        result = 4;
        goto cleanup;
    }

    // L'eau qui arrive dans le premier nœud aval (Stockage) subit déjà la fuite
    // Volume_in dans la récursivité doit être le volume REELLEMENT TRAITÉ par l'usine cible
    recursive_leak_calculation(factory_in_graph->address, initial_volume, &total_leaks);

    // --- 4. Écriture de l'historique ---
    // Le volume doit être en MILLIONS de m³ (le CSV est en k.m³ = 1000 m³)
    // k.m³ -> M.m³ : diviser par 1000
    double leaks_in_million_m3 = total_leaks / 1000.0;

    FILE *f_history = fopen(history_filepath, "a"); // 'a' pour ajouter
    if (f_history == NULL) {
        perror("Erreur lors de l'ouverture du fichier historique");
        result = 5;
        goto cleanup;
    }

    // Le format exact de l'historique dépend de votre choix, mais doit être clair
    fprintf(f_history, "%s;%lf\n", factory_id, leaks_in_million_m3);
    fclose(f_history);

    printf("Pertes calculées pour %s : %.6lf M.m³\n", factory_id, leaks_in_million_m3);

cleanup:
    free_factory_avl(factory_root);
    free_distribution_graph(search_root);
    return result;
}

void free_distribution_graph(AVLSearchNode *root_search) {
    if (root_search == NULL) return;

    // Libération en post-ordre pour l'AVL
    free_distribution_graph(root_search->left);
    free_distribution_graph(root_search->right);

    // Récupérer le nœud du graphe avant de libérer le nœud de l'AVL de recherche
    DistributionNode *node = root_search->address;

    // Libérer le nœud AVL de recherche
    free(root_search->id);
    free(root_search);

    if (node == NULL) return; // Déjà traité ou erreur

    // Libérer la liste chaînée des enfants du nœud du graphe
    ChildLink *current = node->children;
    ChildLink *next;
    while (current != NULL) {
        next = current->next;
        free(current->aval_id); // Libérer l'ID de l'aval du lien
        free(current);
        current = next;
    }

    // Libérer le nœud du graphe lui-même
    free(node->id);
    free(node->factory_id);
    free(node);
}


// Fonction de découpage très simple (ne gère pas les guillemets)
static void tokenize_line(char *line, char **cols) {
    char *token;
    int i = 0;

    // Utiliser strtok pour découper par le séparateur ';'
    token = strtok(line, ";");
    while (token != NULL && i < 5) {
        cols[i] = token;
        token = strtok(NULL, ";");
        i++;
    }
    // Assurer que les colonnes non remplies sont NULL ou une chaîne vide
    while (i < 5) {
        cols[i] = "";
        i++;
    }
}

/** @brief Détermine si une chaîne représente une Source ou une Usine (heuristique simple) */
static int is_factory_or_source(const char *str) {
    return strstr(str, "usine_") == str || strstr(str, "source_") == str;
}

// --- Fonctions de parsing principales ---

int parse_data_for_histogram(const char *filepath, AVLFactoryNode **factory_root) {
    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        perror("Erreur : Impossible d'ouvrir le fichier CSV");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    char *cols[5];
    int line_num = 0;

    // Lire la première ligne (entête) et l'ignorer
    if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
        fclose(f);
        return 1;
    }

    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        line_num++;
        // Remplacer le saut de ligne
        line[strcspn(line, "\n")] = 0;

        // Créer une copie modifiable de la ligne pour strtok
        char *line_copy = strdup(line);
        if (line_copy == NULL) { continue; }

        tokenize_line(line_copy, cols);

        // Cas 1: Ligne Usine (Capacité Max)
        // La colonne 1 (Factory ID) contient un ID d'usine, Colonne 2 est vide.
        if (strstr(cols[COL_FACTORY_ID], "usine_") != NULL && strlen(cols[COL_AMONT_ID]) == 0) {
            double capacity = atof(cols[COL_VOLUME]);
            *factory_root = insert_factory(*factory_root, cols[COL_FACTORY_ID], capacity);
        }

        // Cas 2: Ligne Source -> Usine (Volume Capté, Fuite)
        // La colonne 2 (Amont ID) contient 'source_'
        else if (strstr(cols[COL_AMONT_ID], "source_") != NULL && strstr(cols[COL_AVAL_ID], "usine_") != NULL) {
            double captured_volume = atof(cols[COL_VOLUME]);
            float leak_pct = atof(cols[COL_LEAK_PCT]);

            // L'ID de l'usine est dans la colonne 3 (Aval ID)
            *factory_root = insert_factory(*factory_root, cols[COL_AVAL_ID], 0.0); // Crée le nœud si inexistant
            AVLFactoryNode *factory_node = search_factory(*factory_root, cols[COL_AVAL_ID]);

            if (factory_node != NULL) {
                update_factory_volumes(factory_node, captured_volume, leak_pct);
            }
        }

        free(line_copy);
    }

    fclose(f);
    return 0;
}

int parse_data_for_graph(const char *filepath, AVLFactoryNode *factory_root, AVLSearchNode **search_root) {
    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        perror("Erreur : Impossible d'ouvrir le fichier CSV pour le graphe");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    char *cols[5];
    int result = 0;

    // --- PASSE 1: Création des nœuds (Usine, Stockage, Jonction, Raccordement) ---
    // Lire la première ligne (entête) et l'ignorer
    if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
        fclose(f);
        return 1;
    }

    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        line[strcspn(line, "\n")] = 0;
        char *line_copy = strdup(line);
        if (line_copy == NULL) { continue; }

        tokenize_line(line_copy, cols);

        // Factory ID (dans la colonne 1) est l'ID de l'usine associée au tronçon
        char *factory_id = cols[COL_FACTORY_ID];

        // 1. Gérer les usines déclarées seules (elles sont aussi des nœuds de distribution)
        // Colonne 1 a un ID d'usine, Colonne 2 est vide.
        if (strstr(cols[COL_FACTORY_ID], "usine_") != NULL && strlen(cols[COL_AMONT_ID]) == 0) {
            // Créer le nœud s'il n'est pas déjà dans l'AVL de recherche
            if (search_distribution_node(*search_root, cols[COL_FACTORY_ID]) == NULL) {
                DistributionNode *node = create_distribution_node(cols[COL_FACTORY_ID], cols[COL_FACTORY_ID]);
                if (node != NULL) {
                     *search_root = insert_search_node(*search_root, node->id, node);
                }
            }
        }

        // 2. Gérer les tronçons (Amont -> Aval)
        char *amont_id = cols[COL_AMONT_ID];
        char *aval_id = cols[COL_AVAL_ID];

        // Nœud Amont (si n'est pas une source)
        if (strlen(amont_id) > 0 && strncmp(amont_id, "source_", 7) != 0) {
            // Créer un nœud et l'ajouter à l'AVL de recherche s'il n'existe pas
            if (search_distribution_node(*search_root, amont_id) == NULL) {
                // Utiliser factory_id si disponible, sinon on utilise l'amont ID (cas spécial)
                const char *f_id = (strlen(factory_id) > 0) ? factory_id : amont_id;
                DistributionNode *node = create_distribution_node(amont_id, f_id);
                if (node != NULL) {
                     *search_root = insert_search_node(*search_root, node->id, node);
                }
            }
        }

        // Nœud Aval (si n'est pas un usager)
        if (strlen(aval_id) > 0 && strncmp(aval_id, "usager_", 7) != 0) {
            // Créer un nœud et l'ajouter à l'AVL de recherche s'il n'existe pas
            if (search_distribution_node(*search_root, aval_id) == NULL) {
                const char *f_id = (strlen(factory_id) > 0) ? factory_id : aval_id;
                DistributionNode *node = create_distribution_node(aval_id, f_id);
                if (node != NULL) {
                     *search_root = insert_search_node(*search_root, node->id, node);
                }
            }
        }

        free(line_copy);
    }

    // --- PASSE 2: Création des liens (Tronçons aval) ---
    // Rembobiner le fichier pour la seconde passe
    fseek(f, 0, SEEK_SET);

    // Ignorer l'entête
    if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
        fclose(f);
        return 1;
    }

    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        line[strcspn(line, "\n")] = 0;
        char *line_copy = strdup(line);
        if (line_copy == NULL) { continue; }

        tokenize_line(line_copy, cols);

        // On cherche les lignes décrivant un tronçon (Amont -> Aval)
        // et où l'Amont N'EST PAS une 'source_'
        if (strlen(cols[COL_AMONT_ID]) > 0 && strncmp(cols[COL_AMONT_ID], "source_", 7) != 0) {
            char *amont_id = cols[COL_AMONT_ID];
            char *aval_id = cols[COL_AVAL_ID];
            float leak_pct = atof(cols[COL_LEAK_PCT]);

            // 1. Trouver l'adresse du nœud Parent (Amont)
            AVLSearchNode *parent_search = search_distribution_node(*search_root, amont_id);

            // 2. Trouver l'adresse du nœud Enfant (Aval)
            // L'enfant peut être un 'usager_', qui n'a pas de nœud dans l'AVL de recherche.
            // On peut chercher l'adresse, mais si l'enfant est un usager, child_search sera NULL.
            AVLSearchNode *child_search = search_distribution_node(*search_root, aval_id);

            if (parent_search != NULL && parent_search->address != NULL) {
                // S'assurer que le parent est un nœud de distribution (pas un usager)

                // Si l'enfant est un nœud de distribution (usine, stockage...)
                if (child_search != NULL && child_search->address != NULL) {
                     add_child_link(parent_search->address, child_search->address, leak_pct);
                }
                // Si l'enfant est un usager, on crée un lien temporaire sans nœud cible
                else if (strncmp(aval_id, "usager_", 7) == 0) {
                     // Dans ce cas, on crée un ChildLink qui pointera vers NULL pour child_node.
                     // Le volume est consommé à ce niveau après fuite.
                     DistributionNode *dummy_user_node = create_distribution_node(aval_id, parent_search->address->factory_id);
                     add_child_link(parent_search->address, dummy_user_node, leak_pct);
                     free(dummy_user_node->id);
                     free(dummy_user_node->factory_id);
                     free(dummy_user_node);
                }
            }
        }

        free(line_copy);
    }

    fclose(f);
    return result;
}

static int get_search_balance_factor(AVLSearchNode *node) {
    if (node == NULL) return 0;
    return get_search_height(node->left) - get_search_height(node->right);
}

/** @brief Rotation droite pour AVLSearchNode. */
static AVLSearchNode *search_right_rotate(AVLSearchNode *y) {
    AVLSearchNode *x = y->left;
    AVLSearchNode *T2 = x->right;

    // Effectuer la rotation
    x->right = y;
    y->left = T2;

    // Mise à jour des hauteurs
    update_search_height(y);
    update_search_height(x);

    return x;
}

/** @brief Rotation gauche pour AVLSearchNode. */
static AVLSearchNode *search_left_rotate(AVLSearchNode *x) {
    AVLSearchNode *y = x->right;
    AVLSearchNode *T2 = y->left;

    // Effectuer la rotation
    y->left = x;
    x->right = T2;

    // Mise à jour des hauteurs
    update_search_height(x);
    update_search_height(y);

    return y;
}

// ... (create_search_node est OK)

AVLSearchNode *insert_search_node(AVLSearchNode *node, const char *id, DistributionNode *address) {
    // Insertion standard (voir factory.c pour le détail)
    if (node == NULL) return create_search_node(id, address);
    int cmp = strcmp(id, node->id);

    if (cmp < 0) node->left = insert_search_node(node->left, id, address);
    else if (cmp > 0) node->right = insert_search_node(node->right, id, address);
    else { node->address = address; return node; }

    // Mise à jour de la hauteur et rééquilibrage
    update_search_height(node);
    int balance = get_search_balance_factor(node);

    // Cas de rotations (4 cas standards de l'AVL)
    // Gauche-Gauche (LL)
    if (balance > 1 && strcmp(id, node->left->id) < 0) return search_right_rotate(node);

    // Droite-Droite (RR)
    if (balance < -1 && strcmp(id, node->right->id) > 0) return search_left_rotate(node);

    // Gauche-Droite (LR)
    if (balance > 1 && strcmp(id, node->left->id) > 0) {
        node->left = search_left_rotate(node->left);
        return search_right_rotate(node);
    }

    // Droite-Gauche (RL)
    if (balance < -1 && strcmp(id, node->right->id) < 0) {
        node->right = search_right_rotate(node->right);
        return search_left_rotate(node);
    }

    return node;
}








int main(int argc, char *argv[]) {
    // Vérification des arguments minimales (la vérification complète est faite par le script Shell)
    if (argc < 4) {
        fprintf(stderr, "Erreur : Arguments manquants pour le programme C.\n");
        return 1;
    }

    char *command = argv[1];      // 'histo' ou 'leaks'
    char *arg_value = argv[2];    // 'max', 'src', 'real' ou ID_USINE
    char *csv_filepath = argv[3]; // Chemin du fichier CSV de données

    if (strcmp(command, "histo") == 0) {
        // Le 4ème argument est le nom du fichier de sortie
        if (argc != 5) {
            fprintf(stderr, "Erreur : Argument de fichier de sortie manquant pour histo.\n");
            return 2;
        }
        char *output_filepath = argv[4];

        // Appel de la fonction de traitement d'histogramme
        return process_histogram(csv_filepath, arg_value, output_filepath);

    } else if (strcmp(command, "leaks") == 0) {
        // Le 4ème argument est le nom du fichier historique des fuites
        if (argc != 5) {
            fprintf(stderr, "Erreur : Argument de fichier historique manquant pour leaks.\n");
            return 3;
        }
        char *history_filepath = argv[4];

        // Appel de la fonction de calcul des fuites
        return calculate_leaks(csv_filepath, arg_value, history_filepath);

    } else {
        fprintf(stderr, "Erreur : Commande C non reconnue.\n");
        return 4;
    }
}