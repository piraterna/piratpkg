/******************************************************************************
 * piratpkg.c - A minimal package manager written in C89
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *
 * For more information on Piraterna, visit:
 * https://piraterna.org
 *****************************************************************************/

#include <stdio.h>
#include <parser.h>

int main()
{
    const char* test;
    struct key_value_pair kv_pair;

    test = "repo=example/";
    if (parse_single_key_value(test, &kv_pair) == 0)
    {
        printf("Key: %s\n", kv_pair.key);
        printf("Value: %s\n", kv_pair.value);
        free_single_key_value_pair(&kv_pair);
    }
    else
    {
        printf("Failed to parse key-value pair: %s\n", test);
    }

    return 0;
}
