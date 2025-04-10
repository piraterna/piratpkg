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
#include <sandbox.h>
#include <log.h>
#include <strings.h>

#define MAX_FUNCTIONS 10
#define PATH_BUFFER_SIZE 512

/* =============================================================================
 * Helper functions
 * ========================================================================== */

static int _construct_package_path(const char* branch_path,
                                   const char* package_name, char* buffer,
                                   size_t buffer_size, int is_group)
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

static int _package_exists(const char* package_path)
{
    return (access(package_path, F_OK) == 0);
}

static struct repo_branch* _find_branch_from_name(const char* branch_name)
{
    int i;
    for (i = 0; i < g_config.num_branches; i++)
    {
        if (strcmp(g_config.branches[i].name, branch_name) == 0)
        {
            return &g_config.branches[i];
        }
    }
    return NULL;
}

/* =============================================================================
 * Helper function to retrieve package path based on the package name
 * ========================================================================== */

static char* _pkg_get_path(char* package_name)
{
    char* group_name = NULL;
    char* pkg_name = NULL;
    char* branch_name = NULL;
    int is_group = 0;

    if (package_name[0] == '@')
    {
        is_group = 1;
        package_name++;
    }

    if (is_group)
        group_name = pkg_name;

    char* colon_pos = strchr(package_name, ':');
    char* package_path = (char*)arena_alloc(&g_arena, PATH_BUFFER_SIZE);
    if (package_path == NULL)
    {
        ERROR("Failed to alloc package_path\n");
        return NULL;
    }

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

    /* Handle @group:branch format */
    if (group_name != NULL && branch_name != NULL)
    {
        struct repo_branch* branch = _find_branch_from_name(branch_name);
        if (branch != NULL)
        {
            if (_construct_package_path(branch->path, group_name, package_path,
                                        PATH_BUFFER_SIZE, 1) == 0 &&
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
        struct repo_branch* branch = _find_branch_from_name(branch_name);
        if (branch != NULL)
        {
            if (_construct_package_path(branch->path, pkg_name, package_path,
                                        PATH_BUFFER_SIZE, is_group) == 0 &&
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
        for (i = 0; i < g_config.num_branches; i++)
        {
            struct repo_branch* branch = &g_config.branches[i];
            if (_construct_package_path(branch->path, group_name, package_path,
                                        PATH_BUFFER_SIZE, 1) == 0 &&
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
        for (i = 0; i < g_config.num_branches; i++)
        {
            struct repo_branch* branch = &g_config.branches[i];
            if (_construct_package_path(branch->path, pkg_name, package_path,
                                        PATH_BUFFER_SIZE, is_group) == 0 &&
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
static int _run_normal_callback(struct pkg_ctx* pkg, char** args)
{
    if (args == NULL)
        return ACTION_RET_ERR_UNKNOWN;

    char** arg_ptr = args;
    while (*arg_ptr != NULL)
    {
        if (sandbox_exec(pkg->sandbox, *arg_ptr++, !g_config.verbose) != 0)
            return ACTION_RET_ERR_UNKNOWN;
    }
    return ACTION_RET_OK;
}

static int _run_normal_echo_callback(struct pkg_ctx* pkg, char** args)
{
    if (args == NULL)
        return ACTION_RET_ERR_UNKNOWN;

    char** arg_ptr = args;
    while (*arg_ptr != NULL)
    {
        if (sandbox_exec(pkg->sandbox, *arg_ptr++, false) != 0)
            return ACTION_RET_ERR_UNKNOWN;
    }
    return ACTION_RET_OK;
}

static struct function_entry function_table[] = {
    {"configure", true, _run_normal_callback, NULL},
    {"build", true, _run_normal_callback, NULL},
    {"test", true, _run_normal_callback, NULL},
    {"install", true, _run_normal_callback, NULL},
    {"post_install", true, _run_normal_echo_callback, NULL},
    {"uninstall", true, _run_normal_callback, NULL},
};

/* =============================================================================
 * Function table utilities
 * ========================================================================== */

static struct function_entry* _find_function_by_name(const char* name)
{
    if (name == NULL)
        return NULL;

    int i;
    for (i = 0; i < ARRAY_SIZE(function_table); i++)
    {
        if (strcmp(function_table[i].name, name) == 0)
        {
            return &function_table[i];
        }
    }

    return NULL;
}

struct function_entry* _pkg_find_function(struct pkg_ctx* pkg, const char* name)
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

static int _run_func(struct pkg_ctx* pkg, struct function_entry* func)
{
    if (func == NULL || func->body == NULL)
        return ACTION_RET_ERR_UNKNOWN;
    size_t num_args = 0;
    char* args[MAX_LINE_LENGTH];
    char* line_ptr = strtok(func->body, "\n");

    while (line_ptr != NULL)
    {
        args[num_args++] = line_ptr;
        line_ptr = strtok(NULL, "\n");
    }

    args[num_args] = NULL;

    bool old_v = g_config.verbose;
    g_config.verbose = true;
    MSG("Running %s()...\n", func->name);
    g_config.verbose = old_v;
    return func->callback(pkg, args);
}

/* =============================================================================
 * Helper function to parse function body
 * ========================================================================== */

static int _parse_function_body(FILE* file, const char* func_name,
                                struct function_entry** callback_functions,
                                size_t* num_callbacks)
{
    char line[MAX_LINE_LENGTH];
    size_t body_size = 1024;
    char* body_buffer = (char*)arena_alloc(&g_arena, body_size);
    if (body_buffer == NULL)
    {
        fprintf(stderr,
                "Error: Failed to allocate memory for function body.\n");
        return ACTION_RET_ERR_UNKNOWN;
    }

    size_t body_len = 0;
    int brace_count = 1;

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
                        /* Skip spaces and parentheses */
                        if (!isspace((unsigned char)func_name[i]) &&
                            func_name[i] != '(' && func_name[i] != ')')
                        {
                            cleaned_name[j++] = func_name[i];
                        }
                    }
                    cleaned_name[j] = '\0';
                    func_name = cleaned_name;

                    struct function_entry* func =
                        _find_function_by_name(func_name);

                    if (func == NULL)
                    {
                        WARNING("Unknown function: '%s'\n", func_name);
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
                (char*)arena_realloc(&g_arena, body_buffer, body_size);
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
 * Helper functions for the envp array
 * ========================================================================== */
static int _add_env_var(struct pkg_ctx* pkg, const char* key, const char* value)
{
    if (pkg->num_envp >= 256)
    {
        ERROR("Too many environment variables.\n");
        return ACTION_RET_ERR_UNKNOWN;
    }

    size_t env_var_size =
        strlen(key) + strlen(value) + 2; /* Key + '=' + Value + '\0' */
    pkg->envp[pkg->num_envp] =
        arena_alloc(&g_arena, env_var_size * sizeof(char));
    if (!pkg->envp[pkg->num_envp])
    {
        ERROR("Failed to allocate memory for environment variable.\n");
        return ACTION_RET_ERR_UNKNOWN;
    }

    sprintf(pkg->envp[pkg->num_envp], "%s=%s", key, value);
    pkg->num_envp++;

    return ACTION_RET_OK;
}

/* =============================================================================
 * Public functions
 * ========================================================================== */

struct pkg_ctx* pkg_parse(const char* package_name)
{
    if (package_name == NULL)
        return NULL;

    if (package_name[0] == '@')
    {
        ERROR("We currently do not support groups.\n");
        return NULL;
    }

    struct pkg_ctx* pkg = arena_alloc(&g_arena, sizeof(struct pkg_ctx));
    if (pkg == NULL)
        return NULL;

    if (strlen(package_name) == 0)
    {
        ERROR("Invalid package name.\n");
        return NULL;
    }

    char* package_path = _pkg_get_path((char*)package_name);
    if (package_path == NULL)
    {
        ERROR("Package or group '%s' not found.\n", package_name);
        return NULL;
    }

    /* Open the package file */
    FILE* file = fopen(package_path, "r");
    if (file == NULL)
    {
        ERROR("Failed to open package file %s: %s", package_path,
              strerror(errno));
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    struct function_entry* callback_functions[MAX_FUNCTIONS];
    size_t num_callbacks = 0;

    /* Setup default values for package meta */
    pkg->name = strdup_safe("unkown");
    pkg->description = strdup_safe("unkown");
    pkg->version = strdup_safe("unkown");
    pkg->maintainers = strdup_safe("unkown");

    /* Setup default envp, reallocating as needed */
    _add_env_var(pkg, "PIRATPKG_VERSION", VERSION_STRING);

    /* Process the package file */
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
            else if (strcmp(kv_pair.key, "PACKAGE_DESCRIPTION") == 0)
            {
                pkg->description = kv_pair.value;
            }
            else if (strcmp(kv_pair.key, "PACKAGE_VERSION") == 0)
            {
                pkg->version = kv_pair.value;
            }
            else if (strcmp(kv_pair.key, "PACKAGE_MAINTAINERS") == 0)
            {
                pkg->maintainers = kv_pair.value;
            }
            else if (strcmp(kv_pair.key, "REDIRECT") == 0)
            {
                /* Handle redirects */
                MSG("Package %s redirects to %s\n", package_name,
                    kv_pair.value);
                pkg = pkg_parse(kv_pair.value);
                fclose(file);
                return pkg;
            }

            _add_env_var(pkg, kv_pair.key, kv_pair.value);
        }
        else
        {
            /* If it's a function body, parse the function */
            char* brace_pos = strchr(line, '{');
            if (brace_pos != NULL)
            {
                size_t func_name_len = brace_pos - line;
                char func_name[func_name_len + 1];
                strncpy(func_name, line, func_name_len);
                func_name[func_name_len] = '\0';

                if (_parse_function_body(file, func_name, callback_functions,
                                         &num_callbacks) != 0)
                {
                    ERROR("Failed to parse function body.\n");
                    fclose(file);
                    return NULL;
                }
            }
        }
    }

    fclose(file);

    pkg->functions = callback_functions;
    pkg->num_functions = num_callbacks;
    pkg->branch = NULL;

    /* Add some other env vars to envp */
    _add_env_var(pkg, "PREFIX", g_config.root);

    /* Add NULL to the end of envp, as linux requires */
    pkg->envp[pkg->num_envp] = NULL;

    pkg->sandbox = sandbox_create(pkg->envp);

    return pkg;
}

/* Clean version of pkg_install */
int _pkg_install_clean(struct pkg_ctx* pkg)
{
    size_t i = 0;
    if (pkg == NULL)
    {
        ERROR("Package not found. Installation aborted.\n");
        return ACTION_RET_PKG_ERR_NOT_FOUND;
    }

    STEP("Package: %s-%s\n", pkg->name, pkg->version);
    STEP("Description: %s\n", pkg->description);
    STEP("Maintainers: %s\n", pkg->maintainers);

    for (i = 0; i < pkg->num_functions; i++)
    {
        struct function_entry* func = pkg->functions[i];
        if (strcmp(func->name, "uninstall") != 0)
        {
            if (_run_func(pkg, func) != ACTION_RET_OK)
            {
                ERROR("Function '%s' failed. Installation aborted.\n",
                      func->name);
                return ACTION_RET_ERR_UNKNOWN;
            }
        }
    }
    return 0;
}

int pkg_install(struct pkg_ctx* pkg)
{
    size_t i = 0;
    if (pkg == NULL)
    {
        ERROR("Package not found. Installation aborted.\n");
        return ACTION_RET_PKG_ERR_NOT_FOUND;
    }

    INFO("Package: %s-%s\n", pkg->name, pkg->version);
    INFO("Description: %s\n", pkg->description);
    INFO("Maintainers: %s\n", pkg->maintainers);

    if (g_config.no_confirm == false)
    {
        char user_input;
        printf(COLOR_INFO
               "Do you want to continue installing? [Y/n]: " COLOR_RESET);
        user_input = getchar();

        if (user_input != 'Y' && user_input != 'y' && user_input != '\n')
        {
            INFO("Installation aborted by user.\n");
            return ACTION_RET_OK;
        }
    }
    else
    {
        MSG("Automatic confirmation enabled. Proceeding with "
            "installation...\n");
    }

    INFO("Starting installation...\n");

    /* Run all present functions, except uninstall */
    for (i = 0; i < pkg->num_functions; i++)
    {
        struct function_entry* func = pkg->functions[i];
        if (strcmp(func->name, "uninstall") != 0)
        {
            if (_run_func(pkg, func) != ACTION_RET_OK)
            {
                ERROR("Function '%s' failed. Installation aborted.\n",
                      func->name);
                return ACTION_RET_ERR_UNKNOWN;
            }
        }
    }

    INFO("Installation of %s-%s completed successfully.\n", pkg->name,
         pkg->version);

    /* TODO: Save the installed package in $ROOT/etc/piratpkg/installed or
     * something. */

    sandbox_destroy(pkg->sandbox);
    return ACTION_RET_OK;
}

int pkg_uninstall(struct pkg_ctx* pkg)
{
    if (pkg == NULL)
    {
        ERROR("Package not found. Uninstallation aborted.\n");
        return ACTION_RET_PKG_ERR_NOT_FOUND;
    }

    INFO("Uninstalling: %s-%s\n", pkg->name, pkg->version);
    INFO("Description: %s\n", pkg->description);
    INFO("Maintainers: %s\n", pkg->maintainers);

    if (g_config.no_confirm == false)
    {
        char user_input;
        printf(COLOR_INFO
               "Do you want to continue uninstalling? [Y/n]: " COLOR_RESET);
        user_input = getchar();

        if (user_input != 'Y' && user_input != 'y' && user_input != '\n')
        {
            INFO("Uninstallation aborted by user.\n");
            return ACTION_RET_OK;
        }
    }
    else
    {
        MSG("Automatic confirmation enabled. Proceeding with "
            "uninstallation...\n");
    }

    INFO("Starting uninstallation...\n");

    struct function_entry* uninstall_func =
        _pkg_find_function(pkg, "uninstall");
    if (uninstall_func == NULL)
    {
        WARNING("No uninstall() function defined for this package.\n");
        sandbox_destroy(pkg->sandbox);
        return ACTION_RET_OK;
    }

    if (_run_func(pkg, uninstall_func) != ACTION_RET_OK)
    {
        ERROR("Function 'uninstall' failed. Uninstallation may be "
              "incomplete.\n");
        sandbox_destroy(pkg->sandbox);
        return ACTION_RET_ERR_UNKNOWN;
    }

    INFO("Uninstallation of %s-%s completed successfully.\n", pkg->name,
         pkg->version);

    sandbox_destroy(pkg->sandbox);
    return ACTION_RET_OK;
}
