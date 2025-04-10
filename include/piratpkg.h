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
#include <stdbool.h>

#ifndef _DEV
#define DEFAULT_CONFIG_FILE "/etc/piratpkg/piratpkg.conf"
#else
#define DEFAULT_CONFIG_FILE "piratpkg.conf.test"
#endif /* _DEV */
#define VERSION_STRING "piratpkg 1.0.0-alpha"

/* Helper macros*/
#define ARRAY_SIZE(arr) (int)(sizeof(arr) / sizeof(arr[0]))

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
    char* root;                   /* Root directory */
    int num_branches;             /* Number of available branches */
    char* repo_branches;          /* Available repository branches */
    char* default_branch;         /* Default branch to use */
    struct repo_branch* branches; /* Branch information */
    bool verbose;                 /* Verbose status*/
    bool no_confirm;              /* Auto append yes to questions */
};

struct repo_branch
{
    char* name;
    char* path;
};

/* Globals */
extern struct arena g_arena;
extern struct config g_config;

/* Misc */
#define MAX_LINE_LENGTH 1024

#endif /* PIRATPKG_H */