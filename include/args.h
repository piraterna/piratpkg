/******************************************************************************
 * args.h - Argument parser
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *****************************************************************************/

#ifndef PIRATPKG_ARGS_H
#define PIRATPKG_ARGS_H

/* Return codes for parse_args() */
#define ARG_SUCCESS 0      /* Parsing succeeded */
#define ARG_ERR_UNKNOWN -1 /* Unknown argument encountered */
#define ARG_ERR_MISSING_VALUE \
    -2 /* Argument requires a value but none was provided */
#define ARG_ERR_REQUIRED_MISSING -3 /* Required argument was not provided */
#define ARG_ERR_ALLOC_FAILED -4 /* Memory allocation (e.g., strdup) failed */

struct arg
{
    const char* name;   /* Long name (e.g., "--config") */
    const char* alias;  /* Alias (e.g., "-c") */
    int required;       /* Whether this argument is required */
    char* value;        /* Value of the argument */
    int requires_value; /* Whether this argument requires a value (1 = requires
                           value, 0 = no value) */
};

int parse_args(int argc, char* argv[], struct arg* arg_table, int num_args);
void free_args(struct arg* arg_table, int num_args);

#endif /* PIRATPKG_ARGS_H */
