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

#include <piratpkg.h>
#include <stdio.h>
#include <args.h>
#include <parser.h>
#include <arena.h>
#include <strings.h>

#define DEFAULT_CONFIG_FILE "piratpkg.conf"

struct arena global_arena;

int main(int argc, char** argv)
{
    int status = 0;

    /* 16KB Arena to */
    status = arena_init(&global_arena, DEFAULT_ARENA_SIZE);
    if (status != 0)
    {
        return 1;
    }

    /* Handle arguments */
    struct arg arg_table[] = {
        {"--config", "-c", 0,
         DEFAULT_CONFIG_FILE}, /* --config or -c is not required, defaults
                              to piratpkg.conf */
    };

    int arg_count = sizeof(arg_table) / sizeof(arg_table[0]);
    argc = parse_args(argc, argv, arg_table, arg_count);
    if (argc < 0)
    {
        arena_destroy(&global_arena);
        return 1;
    }

    /* Open and parse config file*/
    if (arg_table[0].value == NULL)
    {
        arg_table[0].value = strdup_safe(DEFAULT_CONFIG_FILE);
    }

    printf("config file: %s\n", arg_table[0].value);
    FILE* config_file =
        fopen(arg_table[0].value, "r"); /* Open config file as read-only */

    (void)config_file;

    /* clean up */
    arena_destroy(&global_arena);
    return 0;
}
