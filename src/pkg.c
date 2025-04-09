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
 * Actions
 * ========================================================================== */

int pkg_install(const char *package_name)
{
    if (package_name == NULL || strlen(package_name) == 0)
    {
        fprintf(stderr, "Error: Invalid package name.\n");
        return ACTION_RET_ERR_UNKNOWN;
    }

    char *package_path = _get_package_path((char *)package_name);

    if (package_path == NULL)
    {
        fprintf(stderr, "Error: Package or group '%s' not found.\n",
                package_name);
        return ACTION_RET_PKG_ERR_NOT_FOUND;
    }

    /* Parse the package or group file */
    bool is_group = false;
    if (package_name[0] == '@')
    {
        is_group = true;
    }

    if (is_group)
    {
        fprintf(stderr, "Error: piratpkg does not yet support groups\n");
        return ACTION_RET_ERR_UNKNOWN;
    }

    printf("installing package @ %s\n", package_path);
    return ACTION_RET_OK;
}
