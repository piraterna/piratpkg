/******************************************************************************
 * arena.c - Fancy arena allocator implementation
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

#include <arena.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void* _arena_malloc(size_t size)
{
    void* ptr = malloc(size);
    if (ptr == NULL)
    {
        fprintf(stderr,
                "Memory allocation failed: Unable to allocate %zu bytes\n",
                size);
    }
    return ptr;
}

int arena_init(struct arena* arena, size_t size)
{
    if (size == 0)
    {
        fprintf(stderr,
                "Arena initialization failed: Size must be greater than 0\n");
        return -1;
    }

    arena->base = _arena_malloc(size);
    if (arena->base == NULL)
    {
        return -1;
    }

    arena->size = size;
    arena->offset = 0;
    return 0;
}

void* arena_alloc(struct arena* arena, size_t size)
{
    if (arena == NULL || arena->base == NULL)
    {
        fprintf(stderr, "Arena not initialized properly\n");
        return NULL;
    }

    if (arena->offset + size > arena->size)
    {
        fprintf(stderr,
                "Arena memory allocation failed: Not enough space to allocate "
                "%zu bytes (%ld > %ld)\n",
                size, arena->offset + size, arena->size);
        return NULL;
    }

    /* Memory allocation */
    void* ptr = (void*)((char*)arena->base + arena->offset);
    arena->offset += size;

    return ptr;
}

void arena_reset(struct arena* arena)
{
    if (arena != NULL)
    {
        arena->offset = 0;
    }
}

void arena_destroy(struct arena* arena)
{
    if (arena != NULL)
    {
        if (arena->base != NULL)
        {
            free(arena->base);
        }
        arena->base = NULL;
        arena->size = 0;
        arena->offset = 0;
    }
}
