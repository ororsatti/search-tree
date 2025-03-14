#ifndef INCLUDE_SEARCH_RADIX_H_
#define INCLUDE_SEARCH_RADIX_H_
#include "utarray.h"
#include "uthash.h"

#define MAX_WORD_SIZE 15
#define LEAF_NODE ""
#define EDGE_NODE ""

typedef struct node {
    char key[MAX_WORD_SIZE];
    struct node* children;
    void* leaf_data;

    UT_hash_handle hh;
} radix_node;

typedef struct {
    char key[MAX_WORD_SIZE];
    int distance;
} search_result;

radix_node* radix_init_root();
UT_array* radix_init_results_array();

radix_node* radix_add(radix_node** root, char* word);
void radix_print(radix_node* root);
radix_node* radix_get(radix_node* root, char* key);
void radix_fuzzy_get(radix_node* root, char* query, int distance, UT_array* results);

void radix_free(radix_node* root);

#endif // INCLUDE_SEARCH_RADIX_H_
