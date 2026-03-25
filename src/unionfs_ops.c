#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "../include/unionfs.h"

/* Forward declarations */
static int unionfs_getattr(const char *path, struct stat *stbuf,
                           struct fuse_file_info *fi);

/* The operations struct — maps FUSE events to your functions.
   Members A and D will fill in more entries (open, readdir, etc.)
   as the project progresses. For now, only getattr is wired. */
static struct fuse_operations unionfs_oper = {
    .getattr = unionfs_getattr,
    /* to be added by team:
    .readdir = unionfs_readdir,
    .open    = unionfs_open,
    .read    = unionfs_read,
    .write   = unionfs_write,
    .create  = unionfs_create,
    .unlink  = unionfs_unlink,
    .mkdir   = unionfs_mkdir,
    .rmdir   = unionfs_rmdir,
    */
};

static int unionfs_getattr(const char *path, struct stat *stbuf,
                           struct fuse_file_info *fi)
{
    (void) fi;  /* not used for getattr */

    char resolved[1024];
    int res;

    /* resolve_path() handles whiteout check + upper/lower lookup.
       It returns -ENOENT if the file is whited out or doesn't exist. */
    res = resolve_path(path, resolved);
    if (res != 0)
        return res;  /* propagates -ENOENT cleanly */

    /* lstat() fills stbuf with file metadata (size, permissions,
       timestamps, etc.) without following symlinks */
    res = lstat(resolved, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}