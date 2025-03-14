#include "libsi.h"
#include "radix.h"
#include "utarray.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

UT_array* sb_init()
{
    UT_icd sb_icd = { sizeof(char),
        NULL,
        NULL,
        NULL };

    UT_array* sb;
    utarray_new(sb, &sb_icd);
    return sb;
}

void sb_append(UT_array* sb, char c)
{
    utarray_push_back(sb, &c);
}

char* sb_to_cstr(UT_array* sb)
{
    if (utarray_len(sb) == 0) {
        return NULL;
    }

    char* str = malloc(sizeof(char) * utarray_len(sb) + 1);

    size_t i = 0;
    for (char* p = (char*)utarray_front(sb);
         p != NULL;
         p = (char*)utarray_next(sb, p)) {

        str[i] = *p;
        i++;
    }

    str[utarray_len(sb)] = '\0';

    return str;
}

void si_search(radix_node* root,
    char* query,
    int max_distance,
    UT_array* results_arr)

{
    radix_fuzzy_get(root, query, max_distance, results_arr);
}

void si_print(radix_node* root)
{
    radix_print(root);
}

void si_free(radix_node* root)
{
    radix_free(root);
}

size_t trim_spaces(char* content, size_t len, size_t cursor)
{
    while (isspace(content[cursor]) && cursor < len) {
        cursor++;
    }

    return cursor;
}

char* get_next_token(char* content, size_t len, size_t* cursor)
{
    UT_array* sb = sb_init();
    // trimming all the spaces to the left
    *cursor = trim_spaces(content, len, *cursor);

    while (*cursor < len) {
        char c = tolower(content[*cursor]);

        // if we hit a space, if so we finish reading a word
        if (isspace(c)) {
            break;
        }
        // when hitting a digit, get all digits and letters
        // example: 2nd, 100th
        if (isdigit(c) == 1) {
            while (*cursor < len) {
                if (isdigit(c) == 1 || isalpha(c) == 1) {
                    sb_append(sb, c);
                    *cursor += 1;
                    c = tolower(content[*cursor]);
                } else {
                    char* token = sb_to_cstr(sb);
                    utarray_free(sb);
                    return token;
                }
            }
        }
        // when hitting a letter, get all letters
        // example: mom, table
        if (isalpha(c) == 1) {
            while (*cursor < len) {
                if (isalpha(c) == 1) {
                    sb_append(sb, c);
                    *cursor += 1;
                    c = tolower(content[*cursor]);
                } else {
                    char* token = sb_to_cstr(sb);
                    utarray_free(sb);
                    return token;
                }
            }
        }
        // if special char take till a break:".", ","
        else {
            sb_append(sb, c);
            *cursor += 1;
            char* token = sb_to_cstr(sb);
            utarray_free(sb);
            return token;
        }
    }
    char* token = sb_to_cstr(sb);
    utarray_free(sb);
    return token;
}

void si_add_document(radix_node** root, char* doc_name, char* content)
{
    size_t cursor = 0;
    size_t doc_len = strlen(content);
    char* token = get_next_token(content, doc_len, &cursor);

    do {
        radix_node* node = radix_get(*root, token);
        if (node != NULL) {
            // update data
            printf("updating data on: %s\n", token);
        } else {
            radix_add(root, token);
        }

        free(token);

        token = get_next_token(content, doc_len, &cursor);
    } while ((token != NULL));
}
