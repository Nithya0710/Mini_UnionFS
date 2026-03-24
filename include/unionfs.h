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

int resolve_path(const char *path, char *resolved_path);

#endif
