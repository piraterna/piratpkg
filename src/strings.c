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
#include <ctype.h>

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

int strcasecmp(const char* s1, const char* s2)
{
    unsigned char c1, c2;

    while (*s1 && *s2)
    {
        c1 = (unsigned char)*s1;
        c2 = (unsigned char)*s2;

        if (c1 != c2)
        {
            if (tolower(c1) != tolower(c2)) return (c1 - c2);
        }

        s1++;
        s2++;
    }

    return (unsigned char)*s1 - (unsigned char)*s2;
}

int count_words(const char* str)
{
    int count = 0;
    int in_word = 0;

    while (*str)
    {
        if (*str == ' ' || *str == '\t' || *str == '\n')
        {
            in_word = 0;
        }
        else if (!in_word)
        {
            in_word = 1;
            count++;
        }
        str++;
    }

    return count;
}
