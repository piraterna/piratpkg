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
#include <sys/stat.h>
#include <log.h>
#include <piratpkg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>

#define TEMP_DIR_BASE "/tmp/sandbox_"

struct sandbox_ctx
{
    char temp_dir[256];
    pid_t pid;
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

struct sandbox_ctx* sandbox_create(void)
{
    struct sandbox_ctx* ctx = arena_alloc(&g_arena, sizeof(struct sandbox_ctx));
    if (_generate_temp_dir(ctx->temp_dir) != 0)
    {
        free(ctx);
        return NULL;
    }

    if (mkdir(ctx->temp_dir, 0700) != 0)
    {
        perror("mkdir");
        return NULL;
    }

    return ctx;
}

/* Thanks to:
 * https://stackoverflow.com/questions/2256945/removing-a-non-empty-directory-programmatically-in-c-or-c
 */
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
        _remove_directory(ctx->temp_dir);
    }
}

int sandbox_exec(struct sandbox_ctx* ctx, const char* command,
                 char* const envp[])
{
    if (ctx == NULL)
    {
        perror("Invalid sandbox context");
        return 1;
    }

    ctx->pid = fork();
    if (ctx->pid < 0)
    {
        perror("fork");
        return 1;
    }
    else if (ctx->pid == 0)
    {
        if (chdir(ctx->temp_dir) < 0)
        {
            perror("chdir");
            return 1;
        }

        char* const args[] = {"/bin/sh", "-c", (char*)command, NULL};
        execve(args[0], args, envp);
        perror("execve");
        return 1;
    }

    int status;
    waitpid(ctx->pid, &status, 0);

    return status;
}
