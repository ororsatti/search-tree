#include "libsi.h"
#include "radix.h"
#include "utarray.h"
#include <stdio.h>

void test_search()
{

    radix_node* root3 = NULL;
    char* doc = "bored boring bored building bow";

    si_add_document(&root3, "test", doc);

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

    radix_node* t = radix_get(root3, "bored");
    printf("%s\n", t->key);
    utarray_free(results);
    si_print(root3);
    si_free(root3);
}

int main(void)
{

    test_search();
    return 0;
}
