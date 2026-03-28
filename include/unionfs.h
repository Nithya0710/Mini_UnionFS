```c
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

/* ================= STATE ================= */

struct mini_unionfs_state {
    char *lower_dir;
    char *upper_dir;
};

/* Access FUSE private data */
#define UNIONFS_DATA ((struct mini_unionfs_state *)fuse_get_context()->private_data)

/* ================= HELPERS ================= */

/* Build full path */
static inline void build_path(char *out, const char *base, const char *path) {
    if (strcmp(path, "/") == 0) {
        snprintf(out, 4096, "%s", base);
    } else {
        snprintf(out, 4096, "%s%s", base, path);
    }
}

/* Check if a file is hidden by whiteout */
static inline int is_whiteout(const char *upper_dir, const char *path)
{
    char wh_path[1024];

    const char *filename = strrchr(path, '/');
    filename = filename ? filename + 1 : path;

    snprintf(wh_path, sizeof(wh_path), "%s/.wh.%s", upper_dir, filename);

    struct stat st;
    return (lstat(wh_path, &st) == 0);
}

/* ================= CORE FUNCTIONS ================= */

/* Path resolution (already implemented) */
int resolve_path(const char *path, char *resolved_path);

/* FUSE operations */
int unionfs_getattr(const char *path, struct stat *stbuf,
                    struct fuse_file_info *fi);

int unionfs_readdir(const char *path, void *buf,
                    fuse_fill_dir_t filler, off_t offset,
                    struct fuse_file_info *fi,
                    enum fuse_readdir_flags flags);

/* ================= OPERATIONS STRUCT ================= */

extern struct fuse_operations unionfs_oper;

#endif
```
