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

typedef void (*function_callback_t)(char **args);

struct function_entry
{
    const char *name;
    bool required;
    function_callback_t callback;
    char *body; /* might be NULL */
};

struct pkg_ctx
{
    char *name;
    char *version;
    char *maintainers;
    struct function_entry **functions;
    size_t num_functions;
};

struct pkg_ctx *pkg_parse(const char *package_name);
int pkg_install(struct pkg_ctx *pkg);

#endif /* PIRATPKG_PKG_H */
