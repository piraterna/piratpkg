/******************************************************************************
 * pkg.h - Package management logic
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *****************************************************************************/

#ifndef PIRATPKG_PKG_H
#define PIRATPKG_PKG_H

#include <stdbool.h>
#include <sandbox.h>

struct pkg_ctx
{
    /* Meta data*/
    char* name;
    char* description;
    char* version;
    char* maintainers;
    char* branch;

    /* Functions */
    struct function_entry** functions;
    size_t num_functions;

    /* Sandbox */
    char* envp[256];
    size_t num_envp;
    struct sandbox_ctx* sandbox;
};

typedef int (*function_callback_t)(struct pkg_ctx* pkg, char** args);

struct function_entry
{
    const char* name;
    bool required;
    function_callback_t callback;
    char* body; /* might be NULL */
};

struct pkg_ctx* pkg_parse(const char* package_name);
int pkg_install(struct pkg_ctx* pkg);
int pkg_uninstall(struct pkg_ctx* pkg);

#endif /* PIRATPKG_PKG_H */
