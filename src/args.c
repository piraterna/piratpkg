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
#include <strings.h>
#include <arena.h>
#include <piratpkg.h>

/* Helper function to match argument names and aliases */
static int match_arg(const char* arg, struct arg* argument)
{
    return (strcmp(arg, argument->name) == 0 ||
            (argument->alias != NULL && strcmp(arg, argument->alias) == 0));
}

/* Helper function to shift arguments left */
static void shift_args(int* argc, char* argv[], int index)
{
    int i;
    for (i = index; i < *argc - 1; i++)
    {
        argv[i] = argv[i + 1];
    }
    (*argc)--;
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
                    if (arg_table[j].value == NULL)
                    {
                        /* Handle flags (arguments without values) */
                        if (arg_table[j].requires_value == 0)
                        {
                            arg_table[j].value = strdup_safe(argv[i]);
                            shift_args(&argc, argv, i);
                            break;
                        }
                    }

                    /* Handle arguments that require a value */
                    if (arg_table[j].requires_value == 1)
                    {
                        if (i + 1 < argc) /* Check if there is a value */
                        {
                            arg_table[j].value = strdup_safe(argv[i + 1]);
                            if (arg_table[j].value == NULL)
                            {
                                return -1;
                            }
                            shift_args(&argc, argv, i);
                            shift_args(&argc, argv, i);
                            i--;
                        }
                        else
                        {
                            /* Error: missing value for argument */
                            printf("Error: Argument '%s' requires a value\n",
                                   argv[i]);
                            return -1;
                        }
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
                    if (arg_table[j].value == NULL)
                    {
                        /* Handle flags (arguments without values) */
                        if (arg_table[j].requires_value == 0)
                        {
                            arg_table[j].value = strdup_safe(argv[i]);
                            shift_args(&argc, argv, i);
                            break;
                        }
                    }

                    /* Handle arguments that require a value */
                    if (arg_table[j].requires_value == 1)
                    {
                        if (i + 1 < argc) /* Check if there is a value */
                        {
                            arg_table[j].value = strdup_safe(argv[i + 1]);
                            if (arg_table[j].value == NULL)
                            {
                                return -1;
                            }
                            shift_args(&argc, argv, i);
                            shift_args(&argc, argv, i);
                            i--;
                        }
                        else
                        {
                            /* Error: missing value for argument */
                            printf("Error: Argument '%s' requires a value\n",
                                   argv[i]);
                            return -1;
                        }
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
                    shift_args(&argc, argv, i);
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

    return argc;
}
