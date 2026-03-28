```c
#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../include/unionfs.h"

/*
 * main.c responsibilities:
 * - Parse arguments
 * - Initialize state (upper/lower dirs)
 * - Pass state to FUSE
 * - Start filesystem
 */

int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <lower_dir> <upper_dir> <mountpoint>\n", argv[0]);
        return 1;
    }

    // Allocate state
    struct mini_unionfs_state *state =
        malloc(sizeof(struct mini_unionfs_state));

    if (!state) {
        perror("malloc");
        return 1;
    }

    // Resolve absolute paths
    state->lower_dir = realpath(argv[1], NULL);
    state->upper_dir = realpath(argv[2], NULL);

    if (!state->lower_dir || !state->upper_dir) {
        fprintf(stderr, "Error: invalid lower_dir or upper_dir\n");
        free(state);
        return 1;
    }

    printf("[mini_unionfs] lower_dir = %s\n", state->lower_dir);
    printf("[mini_unionfs] upper_dir = %s\n", state->upper_dir);
    printf("[mini_unionfs] mountpoint = %s\n", argv[3]);

    /*
     * Prepare FUSE arguments
     * We pass:
     *   argv[0] → program name
     *   argv[3] → mountpoint
     *   "-f"    → run in foreground (easier debugging)
     */
    char *fuse_argv[3];
    fuse_argv[0] = argv[0];
    fuse_argv[1] = argv[3];
    fuse_argv[2] = "-f";

    // Launch FUSE
    int ret = fuse_main(3, fuse_argv, &unionfs_oper, state);

    // Cleanup
    free(state->lower_dir);
    free(state->upper_dir);
    free(state);

    return ret;
}
```

