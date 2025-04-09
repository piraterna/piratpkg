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

/* Defines */
#define DEFAULT_CONFIG_FILE "piratpkg.conf"
#define MAX_LINE_LENGTH 1024 /* Define max line length */

/* Globals */
struct arena global_arena;
struct config global_config;

struct arg arg_table[] = {
    {"--help", "-h", 0, NULL, 0},    /* Special argument*/
    {"--version", "-v", 0, NULL, 0}, /* Special argument*/
    {"--config", "-c", 0, DEFAULT_CONFIG_FILE, 1},
};

/* Actions */
typedef int (*ActionCallback)(const char* arg);

struct action_entry
{
    const char* name;
    int expects_arg;
    ActionCallback callback;
};

/* Help functions */
void print_help()
{
    printf("Usage: piratpkg [OPTION]... ACTION [ARGUMENTS]...\n");
    printf("A minimal package manager for Piraterna's Linux distribution.\n\n");
    printf("Options:\n");
    printf("  -h, --help              display this help and exit\n");
    printf("  -v, --version           output version information and exit\n");
    printf(
        "  -c, --config <file>     use specified configuration file (default: "
        "%s)\n",
        DEFAULT_CONFIG_FILE);
    printf("\n");
    printf("Actions:\n");
    printf("  install <package>       install a package\n");
    printf("  remove  <package>       remove a package\n");
    printf("  update                  update the package index\n");
    printf("  upgrade                 upgrade all installed packages\n");
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

/* Install callback */
int action_install(const char* pkg)
{
    printf("Installing package: %s from repo: %s\n", pkg, global_config.repo);
    return 0;
}

/* Entry point */
int main(int argc, char** argv)
{
    /* 16KB Arena to start with */
    int status = arena_init(&global_arena, DEFAULT_ARENA_SIZE);
    if (status != 0)
    {
        return 1;
    }

    /* Handle arguments */
    int arg_count = sizeof(arg_table) / sizeof(arg_table[0]);
    status = parse_args(argc, argv, arg_table, arg_count);
    if (status < 0)
    {
        if (status == ARG_ERR_REQUIRED_MISSING ||
            status == ARG_ERR_MISSING_VALUE)
        {
            print_help();
        }
        arena_destroy(&global_arena);
        return 1;
    }

    /* Manually shift args after parsing */
    int i, j = 0;
    char** new_argv = arena_alloc(&global_arena, argc * sizeof(char*));

    if (new_argv == NULL)
    {
        perror("piratpkg: Memory allocation failed for new argv");
        arena_destroy(&global_arena);
        return 1;
    }

    for (i = 0; i < argc; i++)
    {
        if (argv[i] != NULL)
        {
            new_argv[j++] = argv[i];
        }
    }

    new_argv[j] = NULL;
    argc = j - 1;
    argv = new_argv;

    /* Handle special args, requires comparing to their own alias or name.
     * For weird reasons */
    if (arg_table[0].value != NULL &&
        (strcmp(arg_table[0].value, arg_table[0].name) == 0 ||
         strcmp(arg_table[0].value, arg_table[0].alias) == 0))
    {
        print_help();
        arena_destroy(&global_arena);
        return 0;
    }

    if (arg_table[1].value != NULL &&
        (strcmp(arg_table[1].value, arg_table[1].name) == 0 ||
         strcmp(arg_table[1].value, arg_table[1].alias) == 0))
    {
        print_help();
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

    /* Config keys */
    global_config.repo = NULL;

    /* Parse the config line by line */
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

        /* Parse the line as a key-value pair */
        /* If it fails it's not a kv-pair, the config should only include
         * kv-pairs anyways */
        static struct key_value_pair kv_pair;
        if (parse_single_key_value(line, &kv_pair) != 0)
        {
            continue;
        }

        /* Get the REPO key from the config*/
        if (strcmp(kv_pair.key, "REPO") == 0)
        {
            global_config.repo =
                arena_alloc(&global_arena, strlen(kv_pair.value));
            strcpy(global_config.repo, kv_pair.value);
        }
    }
    fclose(file);

    /* Check repo */
    if (global_config.repo == NULL)
    {
        printf(
            "piratpkg: No repository path set, make sure to set \"REPO\" in "
            "piratpkg.conf!\n");
        arena_destroy(&global_arena);
        return 1;
    }

    /* Get action */
    if (argc < 1)
    {
        print_help();
        arena_destroy(&global_arena);
        return 1;
    }

    const char* action = argv[1];

    /* Handle action*/
    struct action_entry actions[] = {{"install", 1, action_install}};

    int found = 0;
    int num_actions = sizeof(actions) / sizeof(actions[0]);
    for (i = 0; i < num_actions; i++)
    {
        if (strcmp(action, actions[i].name) == 0)
        {
            const char* arg = NULL;
            if (actions[i].expects_arg)
            {
                if (argc < 2)
                {
                    fprintf(stderr, "piratpkg: '%s' expects an argument\n",
                            action);
                    arena_destroy(&global_arena);
                    return 1;
                }
                arg = argv[2];
            }

            found = 1;
            status = actions[i].callback(arg);
            break;
        }
    }

    if (!found)
    {
        fprintf(stderr, "piratpkg: Unknown action '%s'\n", action);
        print_help();
        arena_destroy(&global_arena);
        return 1;
    }

    /* Clean up */
    arena_destroy(&global_arena);
    return 0;
}
