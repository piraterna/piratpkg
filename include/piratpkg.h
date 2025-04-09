/******************************************************************************
 * piratpkg.h - A minimal package manager written in C89
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *
 * For more information on Piraterna, visit:
 * https://piraterna.org
 ******************************e***********************************************/

#ifndef PIRATPKG_H
#define PIRATPKG_H

#include <arena.h>

/* Action returns */
#define ACTION_RET_OK 0

/* Package-related errors */
#define ACTION_RET_PKG_ERR_NOT_FOUND -1
#define ACTION_RET_PKG_ERR_INVALID_FORMAT -2
#define ACTION_RET_PKG_ERR_DEPENDENCY -3
#define ACTION_RET_PKG_ERR_ALREADY_INSTALLED -4
#define ACTION_RET_PKG_ERR_CONFLICT -5

/* I/O and system-related errors */
#define ACTION_RET_ERR_IO -10
#define ACTION_RET_ERR_PERMISSION -11
#define ACTION_RET_ERR_NO_SPACE -12
#define ACTION_RET_ERR_CONFIG_MISSING -13

/* Generic/unknown error */
#define ACTION_RET_ERR_UNKNOWN -100

/* Config struct and branch struct */
struct config
{
    char *root;              /* Root directory */
    int num_branches;        /* Number of available branches */
    char *repo_branches;     /* Available repository branches */
    char *default_branch;    /* Default branch to use */
    struct branch *branches; /* Branch information */
};

struct branch
{
    char *name;
    char *path;
};

/* Globals */
extern struct arena global_arena;
extern struct config global_config;

#endif /* PIRATPKG_H */