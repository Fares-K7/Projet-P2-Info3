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

AVLFactoryNode *create_factory_node(const char *id);

AVLFactoryNode *insert_factory(AVLFactoryNode *root, const char *id, double capacity);

AVLFactoryNode *search_factory(AVLFactoryNode *root, const char *id);

void update_factory_volumes(AVLFactoryNode *factory_node, double captured_volume, float leak_percentage);

int generate_histogram_data(AVLFactoryNode *root, const char *criteria, const char *filepath);

void free_factory_avl(AVLFactoryNode *root);

typedef struct ChildLink {
    char *aval_id;                 
    float leak_percentage;         
    struct DistributionNode *child_node; 
    struct ChildLink *next;
} ChildLink;

typedef struct DistributionNode {
    char *id;           
    char *factory_id;   
    ChildLink *children; 
    
} DistributionNode;

typedef struct AVLSearchNode {
    char *id; 
    DistributionNode *address; 
    int height;
    struct AVLSearchNode *left;
    struct AVLSearchNode *right;
} AVLSearchNode;

AVLSearchNode *build_distribution_graph(AVLSearchNode *root_search, AVLFactoryNode *factory_root, const char *filepath);

void recursive_leak_calculation(DistributionNode *node, double volume_in, double *total_leak_volume);

int calculate_leaks(const char *csv_filepath, const char *factory_id, const char *history_filepath);

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

LineType parse_csv_line(char *line, char **cols);

int parse_data_for_histogram(const char *filepath, AVLFactoryNode **factory_root);

int parse_data_for_graph(const char *filepath, AVLFactoryNode *factory_root, AVLSearchNode **search_root);

int process_histogram(const char *csv_filepath, const char *criteria, const char *output_filepath) {
    AVLFactoryNode *factory_root = NULL;

    int result = parse_data_for_histogram(csv_filepath, &factory_root);
    if (result != 0) {
        free_factory_avl(factory_root);
        return result;
    }

    result = generate_histogram_data(factory_root, criteria, output_filepath);

    free_factory_avl(factory_root);
    return result;
}

static int get_height(AVLFactoryNode *node) {
    if (node == NULL) return 0;
    return node->height;
}

static int get_balance_factor(AVLFactoryNode *node) {
    if (node == NULL) return 0;
    return get_height(node->left) - get_height(node->right);
}

static void update_height(AVLFactoryNode *node) {
    if (node != NULL) {
        int left_h = get_height(node->left);
        int right_h = get_height(node->right);
        node->height = (left_h > right_h ? left_h : right_h) + 1;
    }
}

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
    
        return create_factory_node(id);
    }
    int compare_result = strcmp(id, node->data.id);

    if (compare_result < 0) {
        node->left = insert_factory(node->left, id, capacity);
    } else if (compare_result > 0) {
        node->right = insert_factory(node->right, id, capacity);
    } else {
        if (capacity > 0.0) {
             node->data.max_capacity = capacity;
        }
        return node; // Ne rien faire ou retourner
    }

    update_height(node);
    int balance = get_balance_factor(node);

    if (balance > 1 && strcmp(id, node->left->data.id) < 0)
        return right_rotate(node);

    if (balance < -1 && strcmp(id, node->right->data.id) > 0)
        return left_rotate(node);

    if (balance > 1 && strcmp(id, node->left->data.id) > 0) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

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

    double treated_volume = captured_volume * (1.0 - leak_percentage / 100.0);

    factory_node->data.total_captured += captured_volume;
    factory_node->data.total_treated += treated_volume;
}

typedef struct {
    char *id;
    double volume;
} FactorySortItem;

static int compare_factory_items(const void *a, const void *b) {
    const FactorySortItem *itemA = (const FactorySortItem *)a;
    const FactorySortItem *itemB = (const FactorySortItem *)b;
    return strcmp(itemB->id, itemA->id);
}

static void collect_factory_data(AVLFactoryNode *root, const char *criteria, FactorySortItem **array, int *index, int *max_size) {
    if (root == NULL) return;

    if (*index >= *max_size) {
        *max_size *= 2;
        *array = (FactorySortItem *)realloc(*array, (*max_size) * sizeof(FactorySortItem));
        if (*array == NULL) {
            perror("Erreur realloc pour FactorySortItem");
            return;
        }
    }

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

    int count = 0;
    int max_size = 50; // Taille initiale
    FactorySortItem *items = (FactorySortItem *)malloc(max_size * sizeof(FactorySortItem));
    if (items == NULL) {
        perror("Erreur d'allocation pour items");
        return 1;
    }

    collect_factory_data(root, criteria, &items, &count, &max_size);

    if (count > 0) {
        items = (FactorySortItem *)realloc(items, count * sizeof(FactorySortItem));
    } else {
        free(items);
        return 1; // Aucun élément à traiter
    }

    qsort(items, count, sizeof(FactorySortItem), compare_factory_items);

    FILE *f = fopen(filepath, "w");
    if (f == NULL) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        free(items);
        return 1;
    }

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

AVLSearchNode *search_distribution_node(AVLSearchNode *root, const char *id) {
    if (root == NULL) return NULL;
    int cmp = strcmp(id, root->id);
    if (cmp == 0) return root;
    if (cmp < 0) return search_distribution_node(root->left, id);
    return search_distribution_node(root->right, id);
}

DistributionNode *create_distribution_node(const char *id, const char *factory_id) {
    DistributionNode *node = (DistributionNode *)malloc(sizeof(DistributionNode));
    if (node == NULL) { /* Handle error */ return NULL; }

    node->id = strdup(id);
    node->factory_id = strdup(factory_id);
    node->children = NULL;

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

    new_link->next = parent->children;
    parent->children = new_link;
}

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

   

    for (ChildLink *link = node->children; link != NULL; link = link->next) {
        float leak_pct = link->leak_percentage;

        double leak_on_segment = volume_per_child * (leak_pct / 100.0);
        *total_leak_volume += leak_on_segment;
        double volume_out = volume_per_child - leak_on_segment;

        if (link->child_node != NULL) {
            recursive_leak_calculation(link->child_node, volume_out, total_leak_volume);
        } else {
        }
    }
}

int calculate_leaks(const char *csv_filepath, const char *factory_id, const char *history_filepath) {
    AVLFactoryNode *factory_root = NULL;
    AVLSearchNode *search_root = NULL;
    int result = 0;
    double total_leaks = 0.0;

    if (parse_data_for_histogram(csv_filepath, &factory_root) != 0) {
        fprintf(stderr, "Erreur de lecture des données amont.\n");
        return 1;
    }

    AVLFactoryNode *target_factory_node = search_factory(factory_root, factory_id);
    if (target_factory_node == NULL) {
        fprintf(stderr, "Erreur : Usine ID '%s' introuvable.\n", factory_id);
        result = 2; // Erreur spécifique : ID inconnu
        goto cleanup;
    }

    double initial_volume = target_factory_node->data.total_treated; // Volume traité réel

    if (parse_data_for_graph(csv_filepath, factory_root, &search_root) != 0) {
        fprintf(stderr, "Erreur de construction du graphe de distribution.\n");
        result = 3;
        goto cleanup;
    }

    AVLSearchNode *factory_in_graph = search_distribution_node(search_root, factory_id);

    if (factory_in_graph == NULL) {
        fprintf(stderr, "Erreur : L'usine n'est pas la racine d'un sous-réseau aval.\n");
        result = 4;
        goto cleanup;
    }


    recursive_leak_calculation(factory_in_graph->address, initial_volume, &total_leaks);

    double leaks_in_million_m3 = total_leaks / 1000.0;

    FILE *f_history = fopen(history_filepath, "a"); 
    if (f_history == NULL) {
        perror("Erreur lors de l'ouverture du fichier historique");
        result = 5;
        goto cleanup;
    }

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

    free_distribution_graph(root_search->left);
    free_distribution_graph(root_search->right);

    DistributionNode *node = root_search->address;

    free(root_search->id);
    free(root_search);

    if (node == NULL) return; 

    ChildLink *current = node->children;
    ChildLink *next;
    while (current != NULL) {
        next = current->next;
        free(current->aval_id); 
        free(current);
        current = next;
    }
    free(node->id);
    free(node->factory_id);
    free(node);
}


static void tokenize_line(char *line, char **cols) {
    char *token;
    int i = 0;
    token = strtok(line, ";");
    while (token != NULL && i < 5) {
        cols[i] = token;
        token = strtok(NULL, ";");
        i++;
    }
    while (i < 5) {
        cols[i] = "";
        i++;
    }
}

static int is_factory_or_source(const char *str) {
    return strstr(str, "usine_") == str || strstr(str, "source_") == str;
}


int parse_data_for_histogram(const char *filepath, AVLFactoryNode **factory_root) {
    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        perror("Erreur : Impossible d'ouvrir le fichier CSV");
        return 1;
    }
    char line[MAX_LINE_LENGTH];
    char *cols[5];
    int line_num = 0;

    if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
        fclose(f);
        return 1;
    }

    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        line_num++;
        line[strcspn(line, "\n")] = 0;

        char *line_copy = strdup(line);
        if (line_copy == NULL) { continue; }
        tokenize_line(line_copy, cols);
        if (strstr(cols[COL_FACTORY_ID], "usine_") != NULL && strlen(cols[COL_AMONT_ID]) == 0) {
            double capacity = atof(cols[COL_VOLUME]);
            *factory_root = insert_factory(*factory_root, cols[COL_FACTORY_ID], capacity);
        }

        else if (strstr(cols[COL_AMONT_ID], "source_") != NULL && strstr(cols[COL_AVAL_ID], "usine_") != NULL) {
            double captured_volume = atof(cols[COL_VOLUME]);
            float leak_pct = atof(cols[COL_LEAK_PCT]);
            *factory_root = insert_factory(*factory_root, cols[COL_AVAL_ID], 0.0);
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

    if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
        fclose(f);
        return 1;
    }

    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        line[strcspn(line, "\n")] = 0;
        char *line_copy = strdup(line);
        if (line_copy == NULL) { continue; }

        tokenize_line(line_copy, cols);
        if (strstr(cols[COL_FACTORY_ID], "usine_") != NULL && strlen(cols[COL_AMONT_ID]) == 0) {
            if (search_distribution_node(*search_root, cols[COL_FACTORY_ID]) == NULL) {
                DistributionNode *node = create_distribution_node(cols[COL_FACTORY_ID], cols[COL_FACTORY_ID]);
                if (node != NULL) {
                     *search_root = insert_search_node(*search_root, node->id, node);
                }
            }
        }


        if (strlen(amont_id) > 0 && strncmp(amont_id, "source_", 7) != 0) {
            if (search_distribution_node(*search_root, amont_id) == NULL) {
                const char *f_id = (strlen(factory_id) > 0) ? factory_id : amont_id;
                DistributionNode *node = create_distribution_node(amont_id, f_id);
                if (node != NULL) {
                     *search_root = insert_search_node(*search_root, node->id, node);
                }
            }
        }

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
    fseek(f, 0, SEEK_SET);

    if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
        fclose(f);
        return 1;
    }

    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        line[strcspn(line, "\n")] = 0;
        char *line_copy = strdup(line);
        if (line_copy == NULL) { continue; }

        tokenize_line(line_copy, cols);

        
        if (strlen(cols[COL_AMONT_ID]) > 0 && strncmp(cols[COL_AMONT_ID], "source_", 7) != 0) {
            char *amont_id = cols[COL_AMONT_ID];
            char *aval_id = cols[COL_AVAL_ID];
            float leak_pct = atof(cols[COL_LEAK_PCT]);

            AVLSearchNode *parent_search = search_distribution_node(*search_root, amont_id);
            AVLSearchNode *child_search = search_distribution_node(*search_root, aval_id);

            if (parent_search != NULL && parent_search->address != NULL) {

                if (child_search != NULL && child_search->address != NULL) {
                     add_child_link(parent_search->address, child_search->address, leak_pct);
                }
                else if (strncmp(aval_id, "usager_", 7) == 0) {
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

static AVLSearchNode *search_right_rotate(AVLSearchNode *y) {
    AVLSearchNode *x = y->left;
    AVLSearchNode *T2 = x->right;

    x->right = y;
    y->left = T2;

    update_search_height(y);
    update_search_height(x);

    return x;
}

static AVLSearchNode *search_left_rotate(AVLSearchNode *x) {
    AVLSearchNode *y = x->right;
    AVLSearchNode *T2 = y->left;

    y->left = x;
    x->right = T2;

    update_search_height(x);
    update_search_height(y);

    return y;
}


AVLSearchNode *insert_search_node(AVLSearchNode *node, const char *id, DistributionNode *address) {
    if (node == NULL) return create_search_node(id, address);
    int cmp = strcmp(id, node->id);

    if (cmp < 0) node->left = insert_search_node(node->left, id, address);
    else if (cmp > 0) node->right = insert_search_node(node->right, id, address);
    else { node->address = address; return node; }

    update_search_height(node);
    int balance = get_search_balance_factor(node);

    if (balance > 1 && strcmp(id, node->left->id) < 0) return search_right_rotate(node);
    if (balance < -1 && strcmp(id, node->right->id) > 0) return search_left_rotate(node);
    if (balance > 1 && strcmp(id, node->left->id) > 0) {
        node->left = search_left_rotate(node->left);
        return search_right_rotate(node);
    }
    if (balance < -1 && strcmp(id, node->right->id) < 0) {
        node->right = search_right_rotate(node->right);
        return search_left_rotate(node);
    }

    return node;
}


int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Erreur : Arguments manquants pour le programme C.\n");
        return 1;
    }

    char *command = argv[1];      
    char *arg_value = argv[2];    
    char *csv_filepath = argv[3]; 

    if (strcmp(command, "histo") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Erreur : Argument de fichier de sortie manquant pour histo.\n");
            return 2;
        }
        char *output_filepath = argv[4];

        return process_histogram(csv_filepath, arg_value, output_filepath);

    } else if (strcmp(command, "leaks") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Erreur : Argument de fichier historique manquant pour leaks.\n");
            return 3;
        }
        char *history_filepath = argv[4];

        return calculate_leaks(csv_filepath, arg_value, history_filepath);

    } else {
        fprintf(stderr, "Erreur : Commande C non reconnue.\n");
        return 4;
    }

}
