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

struct arena global_arena;

int main(int argc, char** argv)
{
    int status = 0;

    /* 1MB Arena should be enough */
    status = arena_init(&global_arena, 1024 * 1024);
    if (status != 0)
    {
        return 1;
    }

    /* Handle arguments */
    struct arg arg_table[] = {
        {"--config", "-c", 0,
         "piratpkg.conf"}, /* --config or -c is not required, defaults to
                              piratpkg.conf */
    };
    int arg_count = sizeof(arg_table) / sizeof(arg_table[0]);
    if (parse_args(argc, argv, arg_table, arg_count) != 0)
    {
        arena_destroy(&global_arena);
        return 1;
    }

    /* Open and parse config file*/
    printf("config file: %s\n", arg_table[0].value);

    /* clean up */
    arena_destroy(&global_arena);
    return 0;
}
