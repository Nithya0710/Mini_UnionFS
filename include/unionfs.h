#ifndef UNIONFS_H
#define UNIONFS_H

#define FUSE_USE_VERSION 31
#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

struct mini_unionfs_state {
    char *lower_dir;
    char *upper_dir;
};

#define UNIONFS_DATA ((struct mini_unionfs_state *)fuse_get_context()->private_data)

static inline void build_path(char *out, const char *base, const char *path) {
    if (strcmp(path, "/") == 0) {
        snprintf(out, 4096, "%s", base);
    } else {
        snprintf(out, 4096, "%s%s", base, path);
    }
}

static inline int is_whiteout(const char *upper_dir, const char *path)
{
    char wh_path[1024];
    /* Extract just the filename from the path */
    const char *filename = strrchr(path, '/');
    filename = filename ? filename + 1 : path;

    snprintf(wh_path, sizeof(wh_path), "%s/.wh.%s", upper_dir, filename);

    struct stat st;
    return (lstat(wh_path, &st) == 0);  /* exists = whiteout active */
}

int resolve_path(const char *path, char *resolved_path);

#endif
