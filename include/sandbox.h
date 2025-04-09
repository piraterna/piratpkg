/******************************************************************************
 * sandbox.h - Handler for running package functions
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *****************************************************************************/

#ifndef PIRATPKG_SANDBOX_H
#define PIRATPKG_SANDBOX_H

/* Note: This is not a sandboxed way to handle the functions */
/* The name is a scam... */

int sandbox_spawn(const char* exec, char* const envp[]);

#endif /* PIRATPKG_SANDBOX_H */
