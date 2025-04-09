/******************************************************************************
 * sandbox.h - Sandbox handler for sandboxing package scripts
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *****************************************************************************/

#ifndef PIRATPKG_SANDBOX_H
#define PIRATPKG_SANDBOX_H

int sandbox_spawn(const char* exec, char* const envp[]);

#endif /* PIRATPKG_SANDBOX_H */
