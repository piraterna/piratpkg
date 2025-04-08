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
