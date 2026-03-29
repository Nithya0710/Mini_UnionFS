#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>

struct mini_unionfs_state {
    char *lower_dir;
    char *upper_dir;
};

#define UNIONFS_DATA ((struct mini_unionfs_state *) fuse_get_context()->private_data)

void build_full_path(char *dest, const char *base, const char *path) {
    strcpy(dest, base);
    if (path[0] != '/') strcat(dest, "/");
    strcat(dest, path);
}

void build_whiteout_path(char *dest, const char *upper, const char *path) {
    char pathcopy1[PATH_MAX], pathcopy2[PATH_MAX];
    strncpy(pathcopy1, path, PATH_MAX - 1);
    strncpy(pathcopy2, path, PATH_MAX - 1);
    char *dir  = dirname(pathcopy1);
    char *base = basename(pathcopy2);
    snprintf(dest, PATH_MAX, "%s%s/.wh.%s", upper,
             (strcmp(dir, "/") == 0) ? "" : dir, base);
}

int has_whiteout(const char *path) {
    char wh_path[PATH_MAX];
    build_whiteout_path(wh_path, UNIONFS_DATA->upper_dir, path);
    struct stat st;
    return (lstat(wh_path, &st) == 0);
}

int resolve_path(const char *path, char *resolved_path, int *is_lower) {
    char upper_path[PATH_MAX], lower_path[PATH_MAX];

    if (strcmp(path, "/") == 0) {
        build_full_path(resolved_path, UNIONFS_DATA->upper_dir, path);
        if (is_lower) *is_lower = 0;
        return 0;
    }

    if (has_whiteout(path)) return -ENOENT;

    build_full_path(upper_path, UNIONFS_DATA->upper_dir, path);
    struct stat st;
    if (lstat(upper_path, &st) == 0) {
        strcpy(resolved_path, upper_path);
        if (is_lower) *is_lower = 0;
        return 0;
    }

    build_full_path(lower_path, UNIONFS_DATA->lower_dir, path);
    if (lstat(lower_path, &st) == 0) {
        strcpy(resolved_path, lower_path);
        if (is_lower) *is_lower = 1;
        return 0;
    }

    return -ENOENT;
}

int unionfs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        char upper_root[PATH_MAX];
        build_full_path(upper_root, UNIONFS_DATA->upper_dir, "/");
        if (lstat(upper_root, stbuf) != 0) {
            char lower_root[PATH_MAX];
            build_full_path(lower_root, UNIONFS_DATA->lower_dir, "/");
            if (lstat(lower_root, stbuf) != 0)
                return -ENOENT;
        }
        return 0;
    }

    char resolved[PATH_MAX];
    int is_lower;
    int res = resolve_path(path, resolved, &is_lower);
    if (res != 0) return res;

    if (lstat(resolved, stbuf) != 0)
        return -errno;

    return 0;
}
