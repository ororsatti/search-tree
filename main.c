#include "utarray.h"
#include "uthash.h"
#include <_string.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_SIZE 15
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

UT_icd search_result_icd = { sizeof(search_result), NULL, NULL, NULL };

node_t* add_node(node_t** parent, char* key, node_t* children)
{
    node_t* node;

    HASH_FIND_STR(*parent, key, node);
    if (node != NULL) {
        return node;
    }

    node = malloc(sizeof *node);
    strcpy(node->key, key);
    node->children = children;

    HASH_ADD_STR(*parent, key, node);

    return node;
}

int is_key_exist(node_t* node, char* key)
{

    if (HASH_COUNT(node) == 0) {
        return 0;
    }

    node_t* tmp;
    HASH_FIND_STR(node, key, tmp);

    return tmp != NULL;
}

void make_node_edge(node_t** parent)
{
    add_node(parent, EDGE_NODE, NULL);
}

int is_edge_node(node_t* node)
{
    return is_key_exist(node->children, EDGE_NODE);
}

void delete_node(node_t** parent, node_t* node)
{
    HASH_DEL(*parent, node);
    free(node);
}

void strrpt(char* p, char c, size_t times)
{
    for (int i = 0; i < times; ++i) {
        strcat(p, " ");
    }
}

void tree_print(node_t* root, size_t depth)
{
    char* padding = malloc(depth);
    strrpt(padding, ' ', depth);

    node_t *item, *tmp;
    HASH_ITER(hh, root, item, tmp)
    {

        if (strcmp(item->key, EDGE_NODE) == 0) {
            continue;
        }

        if (is_key_exist(item->children, EDGE_NODE)) {
            printf("%s%s !!\n", padding, item->key);
        } else {
            printf("%s%s\n", padding, item->key);
        }

        if (HASH_COUNT(item->children) > 0) {
            tree_print(item->children, depth + 1);
        }
    }
    free(padding);
}

void tree_free(node_t* root)
{

    node_t *item, *tmp;
    HASH_ITER(hh, root, item, tmp)
    {
        if (HASH_COUNT(item->children) > 0) {
            tree_free(item->children);
        }

        HASH_DEL(root, item);
        free(item);
    }
}

size_t node_key_cmp(char* src, char* dest)
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

void str_cpy_partial(char* src, char* dest, size_t size)
{
    memcpy(dest, src, size);
    dest[size] = '\0';
}

node_t* tree_create_path(node_t** root, char* key)
{
    node_t** map = root;
    size_t len = strlen(key), start = 0;

    for (size_t i = 1; i <= len; ++i) {

        node_t *node, *tmp;
        HASH_ITER(hh, *map, node, tmp)
        {
            size_t split_idx = node_key_cmp(key, node->key);
            if (split_idx == 0) {
                continue;
            }

            // get the partial key where the splitting happend
            char partial_key[split_idx + 1];
            str_cpy_partial(key, partial_key, split_idx);

            if (split_idx != strlen(node->key)) {
                char parent_key[split_idx + 1];
                str_cpy_partial(node->key, parent_key, split_idx);

                node_t* intermediate = add_node(map, parent_key, NULL);

                // creating the new section

                // adding the new node (the key we want to insert)
                node_t* new_node = add_node(&intermediate->children, &key[split_idx], NULL);
                make_node_edge(&new_node->children);

                // re-adding the existing node (the one that we splitted)
                add_node(
                    &intermediate->children,
                    &node->key[split_idx],
                    node->children);

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

    node_t* node = add_node(map, key, NULL);
    make_node_edge(&node->children);
    return node;
}

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
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void fuzzy_get(node_t* node, char* query, char* prefix, int* v0, int pos, UT_array* results)
{
    int query_len = strlen(query);
    node_t *el, *tmp;

    HASH_ITER(hh, node, el, tmp)
    {

        if (strcmp(el->key, EDGE_NODE) == 0) {
            printf("distance of %s from %s is %d\n", prefix, query, v0[query_len]);

            search_result res = {
                .distance = v0[query_len],
            };
            strcpy(res.key, prefix);

            utarray_push_back(results, &res);
            continue;
        }

        int key_len = strlen(el->key);
        int internal_v0[query_len + 1];
        int v1[query_len + 1];

        memcpy(internal_v0, v0, (query_len + 1) * sizeof(int));

        for (int i = 0; i < key_len; ++i) {
            v1[0] = i + pos;

            for (int j = 0; j < query_len; ++j) {

                if (el->key[i] == query[j]) {
                    v1[j + 1] = internal_v0[j];
                } else {
                    int sub_cost = internal_v0[j];
                    int del_cost = v1[j];
                    int ins_cost = internal_v0[j + 1];

                    v1[j + 1] = min(sub_cost, del_cost, ins_cost) + 1;
                }
            }

            memcpy(internal_v0, v1, (query_len + 1) * sizeof(int));
        }

        char* full_key = concat(prefix, el->key);
        if (HASH_COUNT(el->children) > 0) {
            fuzzy_get(el->children, query, full_key, internal_v0, pos + key_len, results);
        }
        free(full_key);
    }
}

void search(node_t** root, char* query, UT_array* results)
{
    int query_len = strlen(query);
    int* v0

        = malloc((query_len + 1) * sizeof(int));
    for (int j = 0; j <= query_len; ++j) {
        v0[j] = j;
    }

    fuzzy_get(*root, query, "", v0, 1, results);
    free(v0);
}

int main(void)
{
    //
    // printf("ROOT 1: \n");
    // node_t* root1 = NULL;
    // add_node(&root1, "foo", NULL);
    // tree_create_path(&root1, "bar");
    // tree_print(root1, 0);
    // printf("\n");
    //
    // printf("ROOT 2: \n");
    // node_t* root2 = NULL;
    // add_node(&root2, "test", NULL);
    // tree_create_path(&root2, "tester");
    // tree_create_path(&root2, "testing");
    // tree_print(root2, 0);
    // printf("\n");

    printf("ROOT 3: \n");
    node_t* root3 = NULL;
    tree_create_path(&root3, "bored");
    tree_create_path(&root3, "boring");
    tree_create_path(&root3, "bow");
    tree_create_path(&root3, "bold");
    tree_create_path(&root3, "boringly");
    tree_print(root3, 0);

    UT_array* results;
    utarray_new(results, &search_result_icd);
    search(&root3, "border", results);

    for (search_result* p = (search_result*)utarray_front(results);
         p != NULL;
         p = (search_result*)utarray_next(results, p)) {
        printf("%s %d\n", p->key, p->distance);
    }
    printf("\n");

    // tree_free(root1);
    // tree_free(root2);
    tree_free(root3);
    utarray_free(results);

    return 0;
}
