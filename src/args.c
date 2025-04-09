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
#include <log.h>

/* Helper function to match argument names and aliases */
static int match_arg(const char* arg, struct arg* argument)
{
    return (strcmp(arg, argument->name) == 0 ||
            (argument->alias != NULL && strcmp(arg, argument->alias) == 0));
}

/* Main parser function */
int parse_args(int argc, char* argv[], struct arg* arg_table, int num_args)
{
    int i, j = 0;

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
                            argv[i] = NULL;
                            break;
                        }
                    }

                    /* Handle arguments that require a value */
                    if (arg_table[j].requires_value == 1)
                    {
                        if (i + 1 < argc &&
                            argv[i + 1] != NULL) /* Check if there is a value */
                        {
                            arg_table[j].value = strdup_safe(argv[i + 1]);
                            if (arg_table[j].value == NULL)
                            {
                                return ARG_ERR_ALLOC_FAILED;
                            }
                            argv[i] = NULL;
                            argv[i + 1] = NULL;
                            i++;
                            break;
                        }
                        else
                        {
                            /* Error: missing value for argument */
                            ERROR("Argument '%s' requires a value\n", argv[i]);
                            return ARG_ERR_MISSING_VALUE;
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
                        if (arg_table[j].requires_value == 0)
                        {
                            arg_table[j].value = strdup_safe(argv[i]);
                            argv[i] = NULL;
                            break;
                        }
                    }

                    /* Handle arguments that require a value */
                    if (arg_table[j].requires_value == 1)
                    {
                        if (i + 1 < argc &&
                            argv[i + 1] != NULL) /* Check if there is a value */
                        {
                            arg_table[j].value = strdup_safe(argv[i + 1]);
                            if (arg_table[j].value == NULL)
                            {
                                return ARG_ERR_ALLOC_FAILED;
                            }
                            argv[i] = NULL;
                            argv[i + 1] = NULL;
                            i++;
                            break;
                        }
                        else
                        {
                            /* Error: missing value for argument */
                            ERROR("Argument '%s' requires a value\n", argv[i]);
                            return ARG_ERR_MISSING_VALUE;
                        }
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
            /* Report error */
            printf("piratpkg: Missing required argument: %s\n",
                   arg_table[i].name);
            return ARG_ERR_REQUIRED_MISSING;
        }
    }

    return ARG_SUCCESS;
}