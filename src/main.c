#define FUSE_USE_VERSION 31
#include <fuse3/fuse.h>
#include "../include/unionfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int unionfs_getattr(const char *path, struct stat *stbuf,
                            struct fuse_file_info *fi) {
    (void) fi;

    char resolved[4096];
    int ret = resolve_path(path, resolved);
    if (ret != 0) return ret;

    if (lstat(resolved, stbuf) == -1)
        return -errno;

    return 0;
}

static int unionfs_access(const char *path, int mask) {
    char resolved[4096];
    int ret = resolve_path(path, resolved);
    if (ret != 0) return ret;

    if (access(resolved, mask) == -1)
        return -errno;

    return 0;
}

struct fuse_operations unionfs_oper = {
    .getattr  = unionfs_getattr,
    .access   = unionfs_access,
};

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <lower_dir> <upper_dir> <mountpoint>\n", argv[0]);
        return 1;
    }

    struct mini_unionfs_state *state = malloc(sizeof(struct mini_unionfs_state));

    state->lower_dir = realpath(argv[1], NULL);
    state->upper_dir = realpath(argv[2], NULL);

    if (!state->lower_dir || !state->upper_dir) {
        fprintf(stderr, "Error: directories not found\n");
        return 1;
    }

    printf("[mini_unionfs] lower_dir = %s\n", state->lower_dir);
    printf("[mini_unionfs] upper_dir = %s\n", state->upper_dir);
    printf("[mini_unionfs] mountpoint = %s\n", argv[3]);

    char *fuse_argv[3];
    fuse_argv[0] = argv[0];
    fuse_argv[1] = argv[3];
    fuse_argv[2] = "-f";

    int ret = fuse_main(3, fuse_argv, &unionfs_oper, state);

    free(state->lower_dir);
    free(state->upper_dir);
    free(state);

    return ret;
}
