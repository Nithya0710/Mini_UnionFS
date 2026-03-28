
<<<<<<< Member-D-Day-1-updates
#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>

#include "../include/unionfs.h"

// ---------------- GETATTR ----------------
int unionfs_getattr(const char *path, struct stat *stbuf,
                    struct fuse_file_info *fi)
{
    (void) fi;

    char resolved[1024];
    int res = resolve_path(path, resolved);
    if (res != 0)
        return res;

    if (lstat(resolved, stbuf) == -1)
        return -errno;

    return 0;
}

int unionfs_readdir(const char *path, void *buf,
                    fuse_fill_dir_t filler, off_t offset,
                    struct fuse_file_info *fi,
                    enum fuse_readdir_flags flags)
{
    (void) offset;
    (void) fi;
    (void) flags;

    char upper_path[1024], lower_path[1024];
    struct mini_unionfs_state *state = fuse_get_context()->private_data;

    snprintf(upper_path, sizeof(upper_path), "%s%s", state->upper_dir, path);
    snprintf(lower_path, sizeof(lower_path), "%s%s", state->lower_dir, path);

    DIR *dp;
    struct dirent *de;

    // store names
    char *entries[1024];
    int count = 0;

    char *whiteouts[1024];
    int wcount = 0;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    // -------- upper --------
    dp = opendir(upper_path);
    if (dp) {
        while ((de = readdir(dp))) {
            if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
                continue;

            if (!strncmp(de->d_name, ".wh.", 4)) {
                whiteouts[wcount++] = strdup(de->d_name + 4);
                continue;
            }

            entries[count++] = strdup(de->d_name);
        }
        closedir(dp);
    }

    // -------- lower --------
    dp = opendir(lower_path);
    if (dp) {
        while ((de = readdir(dp))) {
            if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
                continue;

            int skip = 0;

            // check duplicate
            for (int i = 0; i < count; i++) {
                if (!strcmp(entries[i], de->d_name)) {
                    skip = 1;
                    break;
                }
            }

            // check whiteout
            for (int i = 0; i < wcount; i++) {
                if (!strcmp(whiteouts[i], de->d_name)) {
                    skip = 1;
                    break;
                }
            }

            if (!skip)
                entries[count++] = strdup(de->d_name);
        }
        closedir(dp);
    }

    // -------- output --------
    for (int i = 0; i < count; i++) {
        filler(buf, entries[i], NULL, 0, 0);
        free(entries[i]);
    }

    for (int i = 0; i < wcount; i++) {
        free(whiteouts[i]);
    }

    return 0;
}

struct fuse_operations unionfs_oper = {
    .getattr = unionfs_getattr,
    .readdir = unionfs_readdir,
};
=======
>>>>>>> main

