/******************************************************************************
 * sandbox.c - Handler for running package functions
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <log.h>
#include <piratpkg.h>

/* TODO: Run this inside some temp dir, dont allow cd to anything outside*/
int sandbox_spawn(const char* exec, char* const envp[])
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }
    else if (pid == 0)
    {
        char* const args[] = {"/bin/sh", "-c", (char*)exec, NULL};
        execve(args[0], args, envp);
        perror("execve");
        return 1;
    }
    int status;
    waitpid(pid, &status, 0);
    return status;
}
