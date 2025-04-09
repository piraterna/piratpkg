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

#ifndef PIRATPKG_H
#define PIRATPKG_H

#include <arena.h>

extern struct arena global_arena;

struct config
{
    char *repo;
};

extern struct config global_config;

#endif /* PIRATPKG_H */