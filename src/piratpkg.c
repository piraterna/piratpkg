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
#include <string.h>
#include <stdlib.h>
#include <piratpkg.h>
#include <args.h>
#include <parser.h>
#include <arena.h>
#include <strings.h>
#include <pkg.h>
#include <log.h>
#include <errno.h>

/* Global State */
struct arena g_arena;
struct config g_config;

/* Argument Table */
struct arg arg_table[] = {
    {"--help", "-h", 0, NULL, 0},
    {"--version", "-v", 0, NULL, 0},
    {"--config", "-c", 0, DEFAULT_CONFIG_FILE, 1},
    {"--verbose", "-V", 0, NULL, 0},
    {"--yes", "-y", 0, NULL, 0},
};

/* Action Definition */
typedef int (*action_callback_t)(const char* arg);

struct action_entry
{
    const char* name;
    int expects_arg;
    action_callback_t callback;
};

/* =============================================================================
 * Help and Version Info
 * ========================================================================== */

void print_help()
{
    printf("Usage: piratpkg [OPTION]... ACTION [ARGUMENTS]...\n");
    printf("A minimal package manager for Piraterna's Linux distribution.\n\n");

    printf("Options:\n");
    printf("  -h, --help              display this help and exit\n");
    printf("  -v, --version           output version information and exit\n");
    printf("  -V, --verbose           enables verbose mode\n");
    printf(
        "  -c, --config <file>     use specified configuration file (default: "
        "%s)\n",
        DEFAULT_CONFIG_FILE);

    printf("\nActions:\n");
    printf("  install <package>       install a package\n");

    printf("\nReport bugs to: <contact@piraterna.org>\n");
    printf("Piraterna home page: <https://piraterna.org>\n");
}

void print_version()
{
    printf("%s\n", VERSION_STRING);
    printf("Copyright (C) 2025 Piraterna\n");
    printf("License Apache-2.0: Apache License version 2.0 "
           "<https://www.apache.org/licenses/LICENSE-2.0>\n");
    printf(
        "This is free software: you are free to change and redistribute it.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n");
}

/* =============================================================================
 * Action Handlers
 * ========================================================================== */

int action_install(const char* pkg)
{
    struct pkg_ctx* p = pkg_parse(pkg);
    if (p == NULL)
        return 1;
    return pkg_install(p);
}

/* =============================================================================
 * Path Handling
 * ========================================================================== */

char* get_full_path(const char* path)
{
    char* full_path =
        arena_alloc(&g_arena, strlen(g_config.root) + strlen(path) + 2);
    if (full_path == NULL)
    {
        perror("piratpkg: Memory allocation failed for path");
        return NULL;
    }
    sprintf(full_path, "%s/%s", g_config.root, path);
    return full_path;
}

/* =============================================================================
 * Configuration Validation
 * ========================================================================== */

int validate_config()
{
    int i, j = 0;
    if (g_config.root == NULL)
    {
        g_config.root = "/";
        WARNING("ROOT is not set in the config, defaulting to '/'\n");
    }

    if (g_config.default_branch == NULL)
    {
        ERROR("DEFAULT_BRANCH is not set in the config\n");
        return 1;
    }

    if (g_config.num_branches == 0)
    {
        ERROR("REPO_BRANCHES is not set or empty\n");
        return 1;
    }

    for (i = 0; i < g_config.num_branches; i++)
    {
        int found = 0;
        for (j = 0; j < g_config.num_branches; j++)
        {
            if (g_config.branches[j].path != NULL)
            {
                if (strcasecmp(g_config.branches[i].name,
                               g_config.branches[j].name) == 0)
                {
                    g_config.branches[i].path =
                        get_full_path(g_config.branches[j].path);
                    found = 1;
                    break;
                }
            }
        }

        if (!found)
        {
            WARNING("Branch \"%s\" does not have a matching path "
                    "definition\n",
                    g_config.branches[i].name);
        }
    }

    return 0;
}

/* =============================================================================
 * Main Entry
 * ========================================================================== */

int main(int argc, char** argv)
{
    int i, j, status;
    char* line_copy;
    FILE* file;
    size_t len;
    char line[MAX_LINE_LENGTH];
    struct key_value_pair kv_pair;

    const char* action;
    const char* arg;
    int found;
    int num_actions;
    struct action_entry actions[] = {
        {"install", 1, action_install},
    };

    /* Initialize arena */
    status = arena_init(&g_arena, DEFAULT_ARENA_SIZE);
    if (status != 0)
    {
        return 1;
    }

    /* Parse arguments */
    status = parse_args(argc, argv, arg_table,
                        sizeof(arg_table) / sizeof(arg_table[0]));
    if (status < 0)
    {
        if (status == ARG_ERR_REQUIRED_MISSING ||
            status == ARG_ERR_MISSING_VALUE)
        {
            print_help();
        }
        arena_destroy(&g_arena);
        return 1;
    }

    /* Filter remaining arguments */
    j = 0;
    {
        char** new_argv = arena_alloc(&g_arena, argc * sizeof(char*));
        if (new_argv == NULL)
        {
            ERROR("Memory allocation failed for new argv\n");
            arena_destroy(&g_arena);
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
    }

    /* Handle --help */
    if (arg_table[0].value != NULL &&
        (strcmp(arg_table[0].value, arg_table[0].name) == 0 ||
         strcmp(arg_table[0].value, arg_table[0].alias) == 0))
    {
        print_help();
        arena_destroy(&g_arena);
        return 0;
    }

    /* Handle --version */
    if (arg_table[1].value != NULL &&
        (strcmp(arg_table[1].value, arg_table[1].name) == 0 ||
         strcmp(arg_table[1].value, arg_table[1].alias) == 0))
    {
        print_version();
        arena_destroy(&g_arena);
        return 0;
    }

    /* Handle --verbose */
    if (arg_table[3].value != NULL &&
        (strcmp(arg_table[3].value, arg_table[3].name) == 0 ||
         strcmp(arg_table[3].value, arg_table[3].alias) == 0))
    {
        g_config.verbose = true;
    }
    else
    {
        g_config.verbose = false;
    }

    /* Handle --yes */
    if (arg_table[4].value != NULL &&
        (strcmp(arg_table[4].value, arg_table[4].name) == 0 ||
         strcmp(arg_table[4].value, arg_table[4].alias) == 0))
    {
        g_config.no_confirm = true;
    }
    else
    {
        g_config.no_confirm = false;
    }

    /* Open config file */
    file = fopen(arg_table[2].value, "r");
    if (file == NULL)
    {
        ERROR("Failed to open config file %s: %s\n", arg_table[2].value,
              strerror(errno));
        arena_destroy(&g_arena);
        return 1;
    }

    g_config.branches = NULL;
    g_config.num_branches = 0;

    /* Parse config */
    while (fgets(line, sizeof(line), file) != NULL)
    {
        len = strlen(line);
        if (line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        line_copy = arena_alloc(&g_arena, len + 1);
        if (line_copy == NULL)
        {
            fclose(file);
            arena_destroy(&g_arena);
            return 1;
        }

        memcpy(line_copy, line, len + 1);

        if (parse_single_key_value(line, &kv_pair) != 0)
        {
            continue;
        }

        /* Parsing ROOT key */
        if (strcmp(kv_pair.key, "ROOT") == 0)
        {
            g_config.root = arena_alloc(&g_arena, strlen(kv_pair.value) + 1);
            if (g_config.root)
            {
                strcpy(g_config.root, kv_pair.value);
            }
        }

        /* Parsing REPO_BRANCHES key */
        if (strcmp(kv_pair.key, "REPO_BRANCHES") == 0)
        {
            g_config.num_branches = count_words(kv_pair.value);
            g_config.branches = arena_alloc(
                &g_arena, sizeof(struct repo_branch) * g_config.num_branches);

            char* token = strtok(kv_pair.value, " ");
            int idx = 0;

            while (token != NULL)
            {
                g_config.branches[idx].name = token;
                idx++;
                token = strtok(NULL, " ");
            }
        }

        /* Parsing DEFAULT_BRANCH key */
        if (strcmp(kv_pair.key, "DEFAULT_BRANCH") == 0)
        {
            g_config.default_branch =
                arena_alloc(&g_arena, strlen(kv_pair.value) + 1);
            strcpy(g_config.default_branch, kv_pair.value);
        }
    }

    /* Second pass to extract branch paths from the config file */
    fseek(file, 0, SEEK_SET);

    while (fgets(line, sizeof(line), file) != NULL)
    {
        len = strlen(line);
        if (line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        line_copy = arena_alloc(&g_arena, len + 1);
        if (line_copy == NULL)
        {
            fclose(file);
            arena_destroy(&g_arena);
            return 1;
        }

        memcpy(line_copy, line, len + 1);

        if (parse_single_key_value(line, &kv_pair) != 0)
        {
            continue;
        }

        /* Check for branch path definitions */
        for (i = 0; i < g_config.num_branches; i++)
        {
            if (strcasecmp(kv_pair.key, g_config.branches[i].name) == 0)
            {
                g_config.branches[i].path =
                    arena_alloc(&g_arena, strlen(kv_pair.value) + 1);
                if (g_config.branches[i].path)
                {
                    strcpy(g_config.branches[i].path, kv_pair.value);
                }
            }
        }
    }

    fclose(file);

    /* Validate config */
    status = validate_config();
    if (status != 0)
    {
        arena_destroy(&g_arena);
        return 1;
    }

    /* Check and dispatch action */
    if (argc < 1)
    {
        print_help();
        arena_destroy(&g_arena);
        return 1;
    }

    action = argv[1];
    found = 0;
    num_actions = sizeof(actions) / sizeof(actions[0]);

    for (i = 0; i < num_actions; i++)
    {
        if (strcmp(action, actions[i].name) == 0)
        {
            arg = NULL;
            if (actions[i].expects_arg)
            {
                if (argc < 2)
                {
                    ERROR("'%s' expects an argument\n", action);
                    arena_destroy(&g_arena);
                    return 1;
                }
                arg = argv[2];
            }

            found = 1;
            status = actions[i].callback(arg);
            if (status != 0)
            {
                arena_destroy(&g_arena);
                return 1;
            }
            break;
        }
    }

    if (!found)
    {
        ERROR("Unknown action '%s'\n", action);
        print_help();
        arena_destroy(&g_arena);
        return 1;
    }

    /* Cleanup */
    arena_destroy(&g_arena);
    return 0;
}
