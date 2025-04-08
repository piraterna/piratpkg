/******************************************************************************
 * args.c - Argument parser
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *****************************************************************************/

#include <args.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <arena.h>
#include <piratpkg.h>

/* Memory-safe strdup implementation using the arena */
static char* strdup_safe(const char* str)
{
    if (str == NULL) return NULL;

    size_t len = strlen(str) + 1;
    char* result = (char*)arena_alloc(&global_arena, len);

    if (result == NULL)
    {
        return NULL;
    }

    memcpy(result, str, len);
    return result;
}

/* Helper function to match argument names and aliases */
static int match_arg(const char* arg, struct arg* argument)
{
    return (strcmp(arg, argument->name) == 0 ||
            (argument->alias != NULL && strcmp(arg, argument->alias) == 0));
}

/* Main parser function */
int parse_args(int argc, char* argv[], struct arg* arg_table, int num_args)
{
    int i, j;

    for (i = 1; i < argc; i++)
    {
        /* Check for '--' style arguments (long form) */
        if (strncmp(argv[i], "--", 2) == 0)
        {
            for (j = 0; j < num_args; j++)
            {
                if (match_arg(argv[i], &arg_table[j]))
                {
                    if (i + 1 < argc)
                    {
                        arg_table[j].value = strdup_safe(argv[i + 1]);
                        if (arg_table[j].value == NULL)
                        {
                            return -1;
                        }
                        i++;
                    }
                    break;
                }
            }
        }
        /* Check for '-' style arguments (short form) */
        else if (argv[i][0] == '-' && argv[i][1] != '\0')
        {
            for (j = 0; j < num_args; j++)
            {
                if (match_arg(argv[i], &arg_table[j]))
                {
                    if (i + 1 < argc)
                    {
                        arg_table[j].value = strdup_safe(argv[i + 1]);
                        if (arg_table[j].value == NULL)
                        {
                            return -1;
                        }
                        i++;
                    }
                    break;
                }
            }
        }

        /* Handle positional arguments (no dashes) */
        else
        {
            for (j = 0; j < num_args; j++)
            {
                if (arg_table[j].value == NULL)
                {
                    arg_table[j].value = strdup_safe(argv[i]);
                    if (arg_table[j].value == NULL)
                    {
                        return -1;
                    }
                    break;
                }
            }
        }
    }

    /* Check for required arguments */
    for (i = 0; i < num_args; i++)
    {
        if (arg_table[i].required && arg_table[i].value == NULL)
        {
            printf("Error: Missing required argument: %s\n", arg_table[i].name);
            return -1;
        }
    }

    return 0;
}
