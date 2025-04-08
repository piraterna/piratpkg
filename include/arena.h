/******************************************************************************
 * arena.h - Fancy arena allocator
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

#ifndef PIRATPKG_ARENA_H
#define PIRATPKG_ARENA_H

#include <stddef.h>

#define DEFAULT_ARENA_SIZE 16384 /* 16KB */

struct arena
{
    void* base;    /* Base of the arena (memory pool) */
    size_t size;   /* Total size of the arena */
    size_t offset; /* Current allocation offset */
};

int arena_init(struct arena* arena, size_t size);
void* arena_alloc(struct arena* arena, size_t size);

/* Reset the arena, effectively "freeing" all memory */
void arena_reset(struct arena* arena);

/* Destroy the arena, freeing the memory pool */
void arena_destroy(struct arena* arena);

#endif /* PIRATPKG_ARENA_H */
