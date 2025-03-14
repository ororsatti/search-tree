#ifndef SI_H
#define SI_H
#include "radix.h"
#include "utarray.h"
#include "uthash.h"
#define MAX_DOC_NAME 25

typedef struct {
    char doc_name[MAX_DOC_NAME];
    int count;

    UT_hash_handle hh;
} si_data;

void si_add_word(radix_node** root, char* word);
void si_search(radix_node* root,
    char* query,
    int max_distance,
    UT_array* results_arr);
void si_free(radix_node* root);
void si_print(radix_node* root);
void si_add_document(radix_node** root, char* doc_name, char* content);

UT_array* sb_init();
void sb_append(UT_array* sb, char c);
char* sb_to_cstr(UT_array* sb);

#endif /* end of include guard: SI_H */
