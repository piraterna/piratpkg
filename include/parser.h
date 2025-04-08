/******************************************************************************
 * parser.h - Package manifest parser
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

#ifndef PIRATPKG_PARSER_H
#define PIRATPKG_PARSER_H

/* Key-Value pair parser */
struct key_value_pair
{
    char* key;   /* NULL-terminated string */
    char* value; /* NULL-terminated string */
};

int parse_single_key_value(const char* input, struct key_value_pair* kv_pair);
void free_single_key_value_pair(struct key_value_pair* kv_pair);

#endif /* PIRATPKG_PARSER_H */
