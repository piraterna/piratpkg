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
#include <string.h>

#define DEFAULT_CONFIG_FILE "/etc/piratpkg/piratpkg.conf"
#define MAX_LINE_LENGTH 1024 /* Define max line length */

struct arena global_arena;

void print_help()
{
    printf("Usage: piratpkg [options]\n");
    printf("Options:\n");
    printf("  -h, --help              Show this help message\n");
    printf("  -c, --config <file>     Specify the config file (default: %s)\n",
           DEFAULT_CONFIG_FILE);
}

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
        {"--help", "-h", 0, NULL, 0},
        {"--config", "-c", 0, DEFAULT_CONFIG_FILE, 1},
    };

    int arg_count = sizeof(arg_table) / sizeof(arg_table[0]);
    argc = parse_args(argc, argv, arg_table, arg_count);
    if (argc < 0)
    {
        arena_destroy(&global_arena);
        return 1;
    }

    /* Handle special args */
    if (arg_table[0].value != NULL)
    {
        print_help();
        arena_destroy(&global_arena);
        return 0;
    }

    /* Open and parse config file */
    FILE* file =
        fopen(arg_table[1].value, "r"); /* Open config file as read-only */
    if (file == NULL)
    {
        perror("piratpkg: Failed to open config file");
        arena_destroy(&global_arena);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        size_t len = strlen(line);
        if (line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        char* line_copy = arena_alloc(&global_arena, len + 1);
        if (line_copy == NULL)
        {
            perror("piratpkg: Memory allocation failed");
            fclose(file);
            arena_destroy(&global_arena);
            return 1;
        }

        memcpy(line_copy, line, len + 1);

        /* Parse the line as a key-value pair, if it fails it's not a kv pair */
        static struct key_value_pair kv_pair;
        if (parse_single_key_value(line, &kv_pair) != 0)
        {
            /* TODO: Actually parse things such as "functions" */
            printf("piratpkg: line is not a key-value pair, skipping.\n");
            continue;
        }

        printf("key=%s, value=%s\n", kv_pair.key, kv_pair.value);
    }
    fclose(file);

    /* Clean up */
    arena_destroy(&global_arena);
    return 0;
}
