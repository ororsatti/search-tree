#include "radix.h"
#include "utarray.h"
#include "uthash.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define EMPTY_PREFIX ""
#define FIRST_MATRIX_ROW 1

UT_icd search_result_icd = { sizeof(search_result), NULL, NULL, NULL };

// helpers
int min(int a, int b, int c)
{
    int t = (a < b) ? a : b;
    return (t < c) ? t : c;
}

void print_arr(int* arr, int len)
{
    for (int i = 0; i < len; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

char* concat(const char* s1, const char* s2)
{
    char* result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void strrpt(char* p, char* c, int times)
{
    for (int i = 0; i < times; ++i) {
        strcat(p, c);
    }
}

void str_cpy_partial(const char* src, char* dest, int size)
{
    memcpy(dest, src, size);
    dest[size] = '\0';
}

size_t key_cmp(char* src, char* dest)
{

    size_t src_len = strlen(src);
    size_t dest_len = strlen(dest);
    size_t idx = 0;

    while (idx < src_len && idx < dest_len) {
        if (src[idx] != dest[idx]) {
            break;
        }
        idx++;
    }

    return idx;
}

radix_node* create_node(char* key, radix_node* children)
{
    radix_node* node = malloc(sizeof(radix_node));
    strcpy(node->key, key);
    node->children = children;

    return node;
}

void add_child(radix_node** parent, radix_node* child)
{
    HASH_ADD_STR(*parent, key, child);
}

radix_node* create_leaf_node(char* node_key, radix_node* children)
{
    radix_node* node = create_node(node_key, children);
    add_child(&node->children, create_node(LEAF_NODE, NULL));
    return node;
}

radix_node* radix_create_path(radix_node** root, char* key)

{
    radix_node** map = root;
    size_t len = strlen(key), start = 0;

    for (size_t i = 1; i <= len; ++i) {

        radix_node *node, *tmp;
        HASH_ITER(hh, *map, node, tmp)
        {
            size_t split_idx = key_cmp(key, node->key);
            if (split_idx == 0) {
                continue;
            }

            // get the partial key where the splitting happend
            char partial_key[split_idx + 1];
            str_cpy_partial(key, partial_key, split_idx);

            if (split_idx != strlen(node->key)) {
                char parent_key[split_idx + 1];
                str_cpy_partial(node->key, parent_key, split_idx);

                // create a new branch
                radix_node* intermediate = create_node(parent_key, NULL);
                add_child(map, intermediate);

                // re-adding the existing node (the one that we splitted)
                add_child(&intermediate->children, create_node(&node->key[split_idx], node->children));

                // deleting the old node
                HASH_DEL(*map, node);
                free(node);

                node = intermediate;
            }

            start = strlen(partial_key);
            key = &key[start];

            len = len - start;
            i = 1;

            map = &node->children;
            break;
        }
    }

    if (len == 0) {
        return *map;
    }

    radix_node* node = create_leaf_node(key, NULL);
    add_child(map, node);

    return node;
}

void calculate_relative_edit_distance(char* key, char* query, int* v0_copy, int pos)
{
    int key_len = strlen(key);
    int query_len = strlen(query);
    int v1[query_len + 1];

    for (int i = 0; i < key_len; ++i) {
        v1[0] = i + pos;

        for (int j = 0; j < query_len; ++j) {

            if (key[i] == query[j]) {
                v1[j + 1] = v0_copy[j];
            } else {
                int sub_cost = v0_copy[j];
                int del_cost = v1[j];
                int ins_cost = v0_copy[j + 1];

                v1[j + 1] = min(sub_cost, del_cost, ins_cost) + 1;
            }
        }

        memcpy(v0_copy, v1, (query_len + 1) * sizeof(int));
    }
}

radix_node* radix_add(radix_node** root, char* word)
{
    return radix_create_path(root, word);
}

radix_node* radix_init_root()
{
    return NULL;
}
radix_node* radix_get_child(radix_node* node, char* key)
{
    if (HASH_COUNT(node->children) == 0) {
        return NULL;
    }

    radix_node* tmp;
    HASH_FIND_STR(node->children, key, tmp);

    return tmp;
}

int radix_has_child(radix_node* node, char* key)
{
    return radix_get_child(node, key) != NULL;
}

void radix_print_recursive(radix_node* root, size_t depth)
{
    char* padding = malloc(depth);
    strrpt(padding, " ", depth);

    radix_node *item, *tmp;
    HASH_ITER(hh, root, item, tmp)
    {

        if (strcmp(item->key, LEAF_NODE) == 0) {
            continue;
            printf(" !! ");
        }

        if (radix_has_child(item, LEAF_NODE)) {
            printf("%s%s !!\n", padding, item->key);
        } else {
            printf("%s%s\n", padding, item->key);
        }

        if (HASH_COUNT(item->children) > 0) {
            radix_print_recursive(item->children, depth + 1);
        }
    }
    free(padding);
}

void radix_print(radix_node* root)
{
    radix_print_recursive(root, 0);
}

void radix_free(radix_node* root)
{

    radix_node *item, *tmp;
    HASH_ITER(hh, root, item, tmp)
    {
        if (HASH_COUNT(item->children) > 0) {
            radix_free(item->children);
        }

        HASH_DEL(root, item);
        free(item);
    }
}

void radix_fuzzy_get_recursive(
    radix_node* node,
    char* query,
    char* prefix,
    int* v0,
    int pos,
    int max_edits,
    UT_array* results)
{
    int query_len = strlen(query);
    radix_node *el, *tmp;

    HASH_ITER(hh, node, el, tmp)
    {
        // hit the edge node, it containes the data about the key
        if (strcmp(el->key, LEAF_NODE) == 0) {

            // if the key is in distance greater than the allowed edits,
            // don't add it to the results list
            if (v0[query_len] > max_edits) {
                continue;
            }

            search_result res = {
                .distance = v0[query_len],
            };
            strcpy(res.key, prefix);

            utarray_push_back(results, &res);
            continue;
        }

        int key_len = strlen(el->key);
        int v0_copy[query_len + 1]; // keeping a copy to not modify the original v0

        memcpy(v0_copy, v0, (query_len + 1) * sizeof(int));

        calculate_relative_edit_distance(el->key, query, v0_copy, pos);

        char* full_key = concat(prefix, el->key);
        int key_edit_distance = v0[key_len];

        if (HASH_COUNT(el->children) > 0 && key_edit_distance <= max_edits) {
            radix_fuzzy_get_recursive(
                el->children,
                query,
                full_key,
                v0_copy,
                pos + key_len,
                max_edits,
                results);
        }
        free(full_key);
    }
}

void radix_fuzzy_get(
    radix_node* root,
    char* query,
    int distance,
    UT_array* results)
{
    int query_len = strlen(query);
    int v0[query_len + 1];

    // initiating the first row in the matrix
    for (int j = 0; j <= query_len; ++j) {
        v0[j] = j;
    }

    radix_fuzzy_get_recursive(
        root,
        query,
        EMPTY_PREFIX,
        v0,
        FIRST_MATRIX_ROW,
        distance,
        results);
}

radix_node* radix_get(radix_node* root, char* key)
{
    radix_node* result = NULL;
    size_t key_len = strlen(key), start = 0;

    for (size_t i = start + 1; i <= key_len; ++i) {
        char partial[i - start + 1];

        str_cpy_partial(key, partial, i - start);

        radix_node* node = NULL;
        HASH_FIND_STR(root, partial, node);

        if (node == NULL) {
            continue;
        }

        if (radix_has_child(node, LEAF_NODE)) {
            return node;
        }

        result = radix_get(node->children, &key[i]);
    }

    return result;
}

UT_array* radix_init_results_array()
{
    UT_array* results;
    utarray_new(results, &search_result_icd);
    return results;
}
