/******************************************************************************
 * sandbox.c - Handler for running package functions
 *
 * Authors:
 *    Kevin Alavik <kevin@alavik.se>
 *
 * Copyright (c) 2025 Piraterna
 * All rights reserved.
 *****************************************************************************/

#define _GNU_SOURCE /* For dprintf */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <log.h>
#include <piratpkg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>

#define TEMP_DIR_BASE "/tmp/sandbox_"

struct sandbox_ctx
{
    char temp_dir[256];
    pid_t pid;
    int shell_stdin;
    int shell_stdout;
};

static int _generate_temp_dir(char* dir_name)
{
    time_t t = time(NULL);
    struct tm* tm_info = localtime(&t);

    sprintf(dir_name, "%s%04d%02d%02d%02d%02d%02d", TEMP_DIR_BASE,
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);

    return 0;
}

struct sandbox_ctx* sandbox_create(char* const envp[])
{
    struct sandbox_ctx* ctx = arena_alloc(&g_arena, sizeof(struct sandbox_ctx));
    int stdin_pipe[2], stdout_pipe[2];

    if (_generate_temp_dir(ctx->temp_dir) != 0)
        return NULL;

    if (mkdir(ctx->temp_dir, 0700) != 0)
    {
        perror("mkdir");
        return NULL;
    }

    if (pipe(stdin_pipe) != 0 || pipe(stdout_pipe) != 0)
    {
        perror("pipe");
        return NULL;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return NULL;
    }
    else if (pid == 0)
    {
        char** new_envp;
        size_t envp_len = 0;
        size_t i;

        /* For simplicity copy over the runners ENV as-well as the one specified
         */
        while (environ[envp_len] != NULL)
        {
            envp_len++;
        }

        new_envp = arena_alloc(&g_arena, (envp_len + 1 + 1) * sizeof(char*));

        for (i = 0; i < envp_len; i++)
        {
            new_envp[i] = environ[i];
        }

        i = 0;
        while (envp[i] != NULL)
        {
            new_envp[envp_len + i] = envp[i];
            i++;
        }

        new_envp[envp_len + i] = NULL;

        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stdout_pipe[1], STDERR_FILENO);

        close(stdin_pipe[1]);
        close(stdout_pipe[0]);

        if (chdir(ctx->temp_dir) < 0)
        {
            perror("chdir");
            return NULL;
        }

        execle("/bin/sh", "sh", NULL, new_envp);
        perror("execle");
        _exit(1);
    }

    close(stdin_pipe[0]);
    close(stdout_pipe[1]);

    ctx->pid = pid;
    ctx->shell_stdin = stdin_pipe[1];
    ctx->shell_stdout = stdout_pipe[0];

    return ctx;
}

/* Remove directory recursively */
int _remove_directory(const char* path)
{
    DIR* d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;

    if (d)
    {
        struct dirent* p;

        r = 0;
        while (!r && (p = readdir(d)))
        {
            int r2 = -1;
            char* buf;
            size_t len;

            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
                continue;

            len = path_len + strlen(p->d_name) + 2;
            buf = arena_alloc(&g_arena, len);

            if (buf)
            {
                struct stat statbuf;

                sprintf(buf, "%s/%s", path, p->d_name);
                if (!stat(buf, &statbuf))
                {
                    if (S_ISDIR(statbuf.st_mode))
                        r2 = _remove_directory(buf);
                    else
                        r2 = unlink(buf);
                }
            }
            r = r2;
        }
        closedir(d);
    }

    if (!r)
        r = rmdir(path);

    return r;
}

void sandbox_destroy(struct sandbox_ctx* ctx)
{
    if (ctx != NULL)
    {
        MSG("Destroying sandbox\n");
        close(ctx->shell_stdin);
        close(ctx->shell_stdout);
        kill(ctx->pid, SIGTERM);
        waitpid(ctx->pid, NULL, 0);
        _remove_directory(ctx->temp_dir);
    }
}

int sandbox_exec(struct sandbox_ctx* ctx, const char* command)
{
    if (!ctx || ctx->shell_stdin == -1 || ctx->shell_stdout == -1)
    {
        fprintf(stderr, "Invalid sandbox context\n");
        return 1;
    }

    MSG("Running: '%s'\n", command);
    dprintf(ctx->shell_stdin, "%s\n", command);
    dprintf(ctx->shell_stdin, "echo __END__\n");

    char buffer[256];
    ssize_t n;
    while ((n = read(ctx->shell_stdout, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[n] = '\0';
        if (strstr(buffer, "__END__") != NULL)
        {
            char* endMarker = strstr(buffer, "__END__");
            *endMarker = '\0'; /* Truncate at marker */
            printf("%s", buffer);
            break;
        }
        printf("%s", buffer);
    }

    return 0;
}
