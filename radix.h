#ifndef INCLUDE_SEARCH_RADIX_H_
#define INCLUDE_SEARCH_RADIX_H_
#include "utarray.h"
#include "uthash.h"

#define MAX_SIZE 15
#define LEAF_NODE ""
#define EDGE_NODE ""

typedef struct node {
    char key[MAX_SIZE];
    struct node* children;

    UT_hash_handle hh;
} node_t;

typedef struct {
    char key[MAX_SIZE];
    int distance;
} search_result;

node_t* radix_init_root();
UT_array* radix_init_results_array();

node_t* radix_add(node_t** root, char* word);
void radix_print(node_t* root);
void radix_fuzzy_get(node_t* root, char* query, int distance, UT_array* results);

void radix_free(node_t* root);

#endif // INCLUDE_SEARCH_RADIX_H_
