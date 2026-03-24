#include "../include/unionfs.h"

int resolve_path(const char *path, char *resolved_path) {
    struct mini_unionfs_state *state = UNIONFS_DATA;

    build_path(resolved_path, state->lower_dir, path);
    return 0;
}
