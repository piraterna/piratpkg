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
    printf("Usage: piratpkg [OPTION]...\n");
    printf("A minimal package manager for Piraterna's Linux distribution.\n\n");
    printf("Options:\n");
    printf("  -h, --help              display this help and exit\n");
    printf("  -v, --version           output version information and exit\n");
    printf(
        "  -c, --config <file>     use specified configuration file (default: "
        "%s)\n",
        DEFAULT_CONFIG_FILE);
    printf("\n");
    printf("Report bugs to: <kevin@alavik.se>\n");
    printf("Piraterna home page: <https://piraterna.org>\n");
}

void print_version()
{
    printf("piratpkg 0.1.0\n");
    printf("Copyright (C) 2025 Piraterna\n");
    printf(
        "License Apache-2.0: Apache License version 2.0 "
        "<https://www.apache.org/licenses/LICENSE-2.0>\n");
    printf(
        "This is free software: you are free to change and redistribute "
        "it.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n");
}

int main(int argc, char** argv)
{
    int status = 0;

    /* 16KB Arena to start with */
    status = arena_init(&global_arena, DEFAULT_ARENA_SIZE);
    if (status != 0)
    {
        return 1;
    }

    /* Handle arguments */
    struct arg arg_table[] = {
        {"--help", "-h", 0, NULL, 0},    /* Special argument*/
        {"--version", "-v", 0, NULL, 0}, /* Special argument*/
        {"--config", "-c", 0, DEFAULT_CONFIG_FILE, 1},
    };

    int arg_count = sizeof(arg_table) / sizeof(arg_table[0]);
    argc = parse_args(argc, argv, arg_table, arg_count);
    if (argc < ARG_SUCCESS)
    {
        if (argc == ARG_ERR_REQUIRED_MISSING || argc == ARG_ERR_MISSING_VALUE)
        {
            print_help();
        }

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

    if (arg_table[1].value != NULL)
    {
        print_version();
        arena_destroy(&global_arena);
        return 0;
    }

    /* Open and parse config file */
    FILE* file =
        fopen(arg_table[2].value, "r"); /* Open config file as read-only */
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

        /* Parse the line as a key-value pair, if it fails it's not a kv
         * pair */
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
