/******************************************************************************
 * pkg.c - Package management logic
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <piratpkg.h>
#include <pkg.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <parser.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>

#define MAX_FUNCTIONS 10

/* =============================================================================
 * Helper functions
 * ========================================================================== */

int _construct_package_path(const char *branch_path, const char *package_name,
                            char *buffer, size_t buffer_size, int is_group)
{
    size_t branch_len = strlen(branch_path);
    size_t pkg_len = strlen(package_name);

    if (branch_len + pkg_len + (is_group ? 7 : 5) > buffer_size)
    {
        return -1;
    }

    strcpy(buffer, branch_path);

    /* Ensure the path ends with a '/' if it doesn't already */
    if (buffer[branch_len - 1] != '/')
    {
        buffer[branch_len] = '/';
        branch_len++;
        buffer[branch_len] = '\0';
    }

    strcat(buffer, package_name);
    strcat(buffer, is_group ? ".group" : ".pkg");

    return 0;
}

int _package_exists(const char *package_path)
{
    return (access(package_path, F_OK) == 0);
}

struct branch *_find_branch_from_name(const char *branch_name)
{
    int i;
    for (i = 0; i < global_config.num_branches; i++)
    {
        if (strcmp(global_config.branches[i].name, branch_name) == 0)
        {
            return &global_config.branches[i];
        }
    }
    return NULL;
}

/* =============================================================================
 * Helper function to retrieve package path based on the package name
 * ========================================================================== */

char *_get_package_path(char *package_name)
{
    char *group_name = NULL;
    char *pkg_name = NULL;
    char *branch_name = NULL;
    int is_group = 0;

    if (package_name[0] == '@')
    {
        is_group = 1;
        package_name++;
    }

    char *colon_pos = strchr(package_name, ':');
    if (colon_pos != NULL)
    {
        *colon_pos = '\0';
        branch_name = colon_pos + 1;
        pkg_name = package_name;
    }
    else
    {
        pkg_name = package_name;
    }

    char *package_path = (char *)arena_alloc(&global_arena, 512);
    if (package_path == NULL)
    {
        return NULL;
    }

    /* Handle @group:branch format */
    if (group_name != NULL && branch_name != NULL)
    {
        struct branch *branch = _find_branch_from_name(branch_name);
        if (branch != NULL)
        {
            if (_construct_package_path(branch->path, group_name, package_path,
                                        512, 1) == 0 &&
                _package_exists(package_path))
            {
                return package_path;
            }
        }
        else
        {
            return NULL;
        }
    }
    /* Handle only branch: */
    else if (branch_name != NULL)
    {
        struct branch *branch = _find_branch_from_name(branch_name);
        if (branch != NULL)
        {
            if (_construct_package_path(branch->path, pkg_name, package_path,
                                        512, is_group) == 0 &&
                _package_exists(package_path))
            {
                return package_path;
            }
        }
        else
        {
            return NULL;
        }
    }
    /* Handle group: (no branch) */
    else if (group_name != NULL)
    {
        /* Check all branches for the group */
        int i;
        for (i = 0; i < global_config.num_branches; i++)
        {
            struct branch *branch = &global_config.branches[i];
            if (_construct_package_path(branch->path, group_name, package_path,
                                        512, 1) == 0 &&
                _package_exists(package_path))
            {
                return package_path;
            }
        }
    }
    else
    {
        /* Check all branches for the package */
        int i;
        for (i = 0; i < global_config.num_branches; i++)
        {
            struct branch *branch = &global_config.branches[i];
            if (_construct_package_path(branch->path, pkg_name, package_path,
                                        512, is_group) == 0 &&
                _package_exists(package_path))
            {
                return package_path;
            }
        }
    }

    return NULL;
}

/* =============================================================================
 * Callback functions
 * ========================================================================== */
void _configure_callback(char **args)
{
    if (args == NULL) return;
    printf("Configure function called with args: \n");
    char **arg_ptr = args;
    while (*arg_ptr != NULL)
    {
        printf("- %s\n", *arg_ptr++);
    }
}

struct function_entry function_table[] = {
    {"configure", true, _configure_callback, NULL}};

/* =============================================================================
 * Function table utilities
 * ========================================================================== */

struct function_entry *find_function_by_name(const char *name)
{
    if (name == NULL) return NULL;
    int i;
    for (i = 0; i < (int)(sizeof(function_table) / sizeof(function_table[0]));
         i++)
    {
        if (strcmp(function_table[i].name, name) == 0)
        {
            return &function_table[i];
        }
    }

    return NULL;
}

struct function_entry *find_function_in_pkg(struct pkg_ctx *pkg,
                                            const char *name)
{
    if (pkg == NULL || name == NULL || pkg->functions == NULL)
    {
        return NULL;
    }

    size_t i;
    for (i = 0; i < pkg->num_functions; i++)
    {
        if (strcmp(pkg->functions[i]->name, name) == 0)
        {
            return pkg->functions[i];
        }
    }

    return NULL;
}

void run_function(struct function_entry *func)
{
    if (func == NULL || func->body == NULL) return;
    size_t num_args = 0;
    char *args[MAX_LINE_LENGTH];
    char *line_ptr = strtok(func->body, "\n");

    while (line_ptr != NULL)
    {
        args[num_args++] = line_ptr;
        line_ptr = strtok(NULL, "\n");
    }

    args[num_args] = NULL;
    func->callback(args);
}

/* =============================================================================
 * Helper function to parse key-value pairs
 * ========================================================================== */
int _parse_kv_pairs(FILE *file)
{
    char line[MAX_LINE_LENGTH];
    struct key_value_pair kv_pair;
    size_t len;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        len = strlen(line);
        if (line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        if (parse_single_key_value(line, &kv_pair) == 0)
        {
            /* Todo store them in some package context */
            printf("key=%s, value=%s\n", kv_pair.key, kv_pair.value);
        }
    }
    return 0;
}

/* =============================================================================
 * Helper function to parse function body
 * ========================================================================== */

int _parse_function_body(FILE *file, const char *func_name,
                         struct function_entry **callback_functions,
                         size_t *num_callbacks)
{
    char line[MAX_LINE_LENGTH];
    size_t body_size = 1024;
    char *body_buffer = (char *)arena_alloc(&global_arena, body_size);
    if (body_buffer == NULL)
    {
        fprintf(stderr,
                "Error: Failed to allocate memory for function body.\n");
        return ACTION_RET_ERR_UNKNOWN;
    }

    size_t body_len = 0;
    int brace_count = 1;
    *num_callbacks = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        size_t len = strlen(line);
        size_t start = 0;

        while (start < len && isspace(line[start]))
        {
            start++;
        }

        memmove(line, line + start, len - start + 1);

        size_t i;
        for (i = 0; i < len - start; ++i)
        {
            if (line[i] == '{')
            {
                brace_count++;
            }
            else if (line[i] == '}')
            {
                brace_count--;
                if (brace_count == 0)
                {
                    body_buffer[body_len] = '\0';

                    char cleaned_name[strlen(func_name) + 1];
                    int i, j = 0;
                    for (i = 0; func_name[i] != '\0'; i++)
                    {
                        if (!isspace((unsigned char)func_name[i]))
                        {
                            cleaned_name[j++] = func_name[i];
                        }
                    }
                    cleaned_name[j] = '\0';
                    func_name = cleaned_name;

                    struct function_entry *func =
                        find_function_by_name(func_name);

                    if (func == NULL)
                    {
                        fprintf(stderr, "Warning: Unknown function: '%s'\n",
                                func_name);
                    }
                    else
                    {
                        if (*num_callbacks < MAX_FUNCTIONS)
                        {
                            func->body = body_buffer;
                            callback_functions[*num_callbacks] = func;
                            (*num_callbacks)++;
                        }
                        else
                        {
                            fprintf(
                                stderr,
                                "Warning: Max number of callbacks reached.\n");
                        }
                    }
                    return 0;
                }
            }
        }

        if (body_len + len - start + 1 > body_size)
        {
            body_size *= 2;
            body_buffer =
                (char *)arena_realloc(&global_arena, body_buffer, body_size);
            if (body_buffer == NULL)
            {
                fprintf(
                    stderr,
                    "Error: Failed to reallocate memory for function body.\n");
                return ACTION_RET_ERR_UNKNOWN;
            }
        }

        strcat(body_buffer, line);
        body_len += len - start;
    }

    return ACTION_RET_ERR_UNKNOWN;
}

/* =============================================================================
 * Actions
 * ========================================================================== */

struct pkg_ctx *pkg_parse(const char *package_name)
{
    struct pkg_ctx *pkg = arena_alloc(&global_arena, sizeof(struct pkg_ctx));
    if (pkg == NULL) return NULL;

    if (package_name == NULL || strlen(package_name) == 0)
    {
        fprintf(stderr, "Error: Invalid package name.\n");
        return NULL;
    }

    char *package_path = _get_package_path((char *)package_name);

    if (package_path == NULL)
    {
        fprintf(stderr, "Error: Package or group '%s' not found.\n",
                package_name);
        return NULL;
    }

    /* Open the package file */
    FILE *file = fopen(package_path, "r");
    if (file == NULL)
    {
        perror("piratpkg: Failed to open package file");
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    struct function_entry *callback_functions[MAX_FUNCTIONS];
    size_t num_callbacks = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        size_t len = strlen(line);
        if (line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        struct key_value_pair kv_pair;
        if (parse_single_key_value(line, &kv_pair) == 0)
        {
            /* Get package meta from keys */
            if (strcmp(kv_pair.key, "PACKAGE_NAME") == 0)
            {
                pkg->name = kv_pair.value;
            }
            else if (strcmp(kv_pair.key, "PACKAGE_VERSION") == 0)
            {
                pkg->version = kv_pair.value;
            }
            else if (strcmp(kv_pair.key, "PACKAGE_MAINTAINERS") == 0)
            {
                pkg->maintainers = kv_pair.value;
            }
            else
            {
                printf("Warning: Unknown key: '%s' found in package manifest\n",
                       kv_pair.key);
            }
        }
        else
        {
            /* If it's a function body, parse the function */
            char *brace_pos = strchr(line, '{');
            if (brace_pos != NULL)
            {
                size_t func_name_len = brace_pos - line;
                char func_name[func_name_len + 1];
                strncpy(func_name, line, func_name_len);
                func_name[func_name_len] = '\0';

                if (_parse_function_body(file, func_name, callback_functions,
                                         &num_callbacks) != 0)
                {
                    fprintf(stderr, "Error: Failed to parse function body.\n");
                    fclose(file);
                    return NULL;
                }
            }
        }
    }

    fclose(file);

    pkg->functions = callback_functions;
    pkg->num_functions = num_callbacks;

    return pkg;
}

int pkg_install(struct pkg_ctx *pkg)
{
    if (pkg == NULL)
    {
        fprintf(stderr, "Error: Package not found.\n");
        return ACTION_RET_PKG_ERR_NOT_FOUND;
    }

    printf("installing %s-%s...\n", pkg->name, pkg->version);
    printf("Maintainers: %s\n", pkg->maintainers);

    /* First we configure, if configure is present */
    struct function_entry *configure = find_function_in_pkg(pkg, "configure");
    run_function(
        configure); /* Will just exit and do nothing if configure is NULL */

    return ACTION_RET_OK;
}