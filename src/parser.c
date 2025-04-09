/******************************************************************************
 * parser.c - Package manifest parser
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *****************************************************************************/

#include <parser.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arena.h>
#include <piratpkg.h>

/* Key-Value pair parser */
int parse_single_key_value(const char* input, struct key_value_pair* kv_pair)
{
    char* delimiter;
    size_t key_len, value_len;

    /* Validate input */
    if (input == NULL || kv_pair == NULL)
    {
        return -1;
    }

    /* Find the delimiter '=' */
    delimiter = strchr(input, '=');
    if (delimiter == NULL)
    {
        return -1;
    }

    key_len = delimiter - input;
    value_len = strlen(input) - key_len - 1;

    kv_pair->key = (char*)arena_alloc(&g_arena, key_len + 1);
    kv_pair->value = (char*)arena_alloc(&g_arena, value_len + 1);

    if (kv_pair->key == NULL || kv_pair->value == NULL)
    {
        return -1;
    }

    /* Copy the key and value strings */
    strncpy(kv_pair->key, input, key_len);
    kv_pair->key[key_len] = '\0';

    strncpy(kv_pair->value, delimiter + 1, value_len);
    kv_pair->value[value_len] = '\0';

    return 0;
}
