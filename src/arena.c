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
#include <log.h>
#include <errno.h>

/* Internal utility functions */
static void* _arena_malloc(size_t size)
{
    void* ptr = malloc(size);
    if (ptr == NULL)
    {
        ERROR("Memory allocation failed: %s", strerror(errno));
    }
    return ptr;
}

static void* _arena_realloc(void* ptr, size_t size)
{
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL)
    {
        ERROR("Memory reallocation failed: %s", strerror(errno));
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
    return 0;
}

/* Public functions */
int arena_init(struct arena* arena, size_t size)
{
    if (size == 0)
    {
        ERROR("Arena initialization failed: Size must be greater "
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
        ERROR("Arena not initialized properly\n");
        return NULL;
    }

    if (arena->offset + size > arena->size)
    {
        if (_arena_grow(arena, size) != 0)
        {
            ERROR("Arena memory allocation failed: Unable to grow "
                  "arena\n");
            return NULL;
        }
    }

    /* Memory allocation */
    void* ptr = (void*)((char*)arena->base + arena->offset);
    arena->offset += size;

    return ptr;
}

/* Reallocate a block of memory within the arena */
void* arena_realloc(struct arena* arena, void* ptr, size_t new_size)
{
    if (arena == NULL || arena->base == NULL || ptr == NULL)
    {
        ERROR("Invalid parameters for arena_realloc\n");
        return NULL;
    }

    /* Check if the requested size is within the current arena bounds */
    size_t current_offset = (char*)ptr - (char*)arena->base;
    if (current_offset + new_size <= arena->size)
    {
        /* If the new size fits within the arena, just adjust the offset */
        arena->offset = current_offset + new_size;
        return ptr;
    }
    else
    {
        /* If the new size doesn't fit, grow the arena */
        if (_arena_grow(arena, new_size) != 0)
        {
            return NULL;
        }

        /* Memory allocation */
        void* new_ptr = (char*)arena->base + current_offset;
        arena->offset = current_offset + new_size;
        return new_ptr;
    }
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
