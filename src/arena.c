/******************************************************************************
 * arena.c - Fancy arena allocator
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

/* Internal utility functions */
static void* _arena_malloc(size_t size)
{
    void* ptr = malloc(size);
    if (ptr == NULL)
    {
        perror("piratpkg: Memory allocation failed");
    }
    return ptr;
}

static void* _arena_realloc(void* ptr, size_t size)
{
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL)
    {
        perror("piratpkg: Memory reallocation failed");
    }
    return new_ptr;
}

static int _arena_grow(struct arena* arena, size_t size_needed)
{
    size_t old_size = arena->size;
    size_t new_size = old_size;

    /* Grow arena in increments of DEFAULT_ARENA_SIZE until it is large enough
     */
    while (new_size < arena->offset + size_needed)
    {
        new_size += DEFAULT_ARENA_SIZE;
    }

    void* new_base = _arena_realloc(arena->base, new_size);
    if (new_base == NULL)
    {
        return -1;
    }

    arena->base = new_base;
    arena->size = new_size;

    printf("debug: Grew arena from %zu to %zu\n", old_size, new_size);
    return 0;
}

/* Public functions */
int arena_init(struct arena* arena, size_t size)
{
    if (size == 0)
    {
        fprintf(stderr,
                "piratpkg: Arena initialization failed: Size must be greater "
                "than 0\n");
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
        fprintf(stderr, "piratpkg: Arena not initialized properly\n");
        return NULL;
    }

    if (arena->offset + size > arena->size)
    {
        if (_arena_grow(arena, size) != 0)
        {
            fprintf(stderr,
                    "piratpkg: Arena memory allocation failed: Unable to grow "
                    "arena\n");
            return NULL;
        }
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
