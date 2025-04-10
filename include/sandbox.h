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

struct sandbox_ctx;

struct sandbox_ctx* sandbox_create(char* const envp[]);
void sandbox_destroy(struct sandbox_ctx* ctx);
int sandbox_exec(struct sandbox_ctx* ctx, const char* command, bool silent);

#endif /* PIRATPKG_SANDBOX_H */
