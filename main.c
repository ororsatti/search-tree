#include "radix.h"
#include <stdio.h>

void test_search()
{

    node_t* root3 = NULL;
    char* words[] = {
        "bored",
        "boring",
        "bow",
        "bold",
        "boringly"
    };

    for (int i = 0; i < 5; i++) {
        radix_add(&root3, words[i]);
    }

    UT_array* results = radix_init_results_array();

    printf("searching 'border', max distance: 3\n");
    radix_fuzzy_get(root3, "border", 3, results);

    printf("results: %d\n", utarray_len(results));
    for (search_result* p = (search_result*)utarray_front(results);
         p != NULL;
         p = (search_result*)utarray_next(results, p)) {
        printf("%s %d\n", p->key, p->distance);
    }
    printf("\n");

    radix_print(root3);
    utarray_free(results);
    radix_free(root3);
}

int main(void)
{

    test_search();
    return 0;
}
