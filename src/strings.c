/******************************************************************************
 * strings.c - String manipulation functions
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *****************************************************************************/

#include <strings.h>
#include <piratpkg.h>
#include <stddef.h>
#include <string.h>

char* strdup_safe(const char* str)
{
    if (str == NULL) return NULL;

    size_t len = strlen(str) + 1;
    char* result = (char*)arena_alloc(&global_arena, len);

    if (result == NULL)
    {
        return NULL;
    }

    memcpy(result, str, len);
    return result;
}