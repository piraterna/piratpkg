/******************************************************************************
 * strings.h - String manipulation functions
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *****************************************************************************/

#ifndef PIRATPKG_STRINGS_H
#define PIRATPKG_STRINGS_H

char* strdup_safe(const char* str);
int strcasecmp(const char* s1, const char* s2);
int count_words(const char* str);

#endif /* PIRATPKG_STRINGS_H */