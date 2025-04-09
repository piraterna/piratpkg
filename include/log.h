/******************************************************************************
 * log.h - Logger
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *****************************************************************************/

#ifndef PIRATPKG_LOG_H
#define PIRATPKG_LOG_H

#include <piratpkg.h>
#include <stdio.h>

#define COLOR_RESET "\033[0m"
#define COLOR_WARNING "\033[33m" /* Yellow */
#define COLOR_ERROR "\033[31m"   /* Red */
#define COLOR_INFO "\033[36m"    /* Cyan */
#define COLOR_MSG "\033[90m"     /* Light Black (Gray) */

#define WARNING(fmt, ...)                                                      \
    do                                                                         \
    {                                                                          \
        fprintf(stderr, COLOR_WARNING "<-- Warning: " fmt COLOR_RESET,         \
                ##__VA_ARGS__);                                                \
    } while (0)

#define ERROR(fmt, ...)                                                        \
    do                                                                         \
    {                                                                          \
        fprintf(stderr, COLOR_ERROR "<-- Error: " fmt COLOR_RESET,             \
                ##__VA_ARGS__);                                                \
    } while (0)

#define INFO(fmt, ...)                                                         \
    do                                                                         \
    {                                                                          \
        printf(COLOR_INFO "==> " COLOR_RESET fmt, ##__VA_ARGS__);              \
    } while (0)

#define MSG(fmt, ...)                                                          \
    do                                                                         \
    {                                                                          \
        if (g_config.verbose)                                                  \
            printf(COLOR_MSG "---> " COLOR_RESET fmt, ##__VA_ARGS__);          \
    } while (0)

#endif /* PIRATPKG_LOG_H */
