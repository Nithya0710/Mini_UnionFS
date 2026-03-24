#include "../include/unionfs.h"

/*
 * resolve_path()
 *
 * Given a virtual path (e.g. "/config.txt"), figure out
 * where the actual file lives. Check in this exact order:
 *
 * 1. upper_dir/.wh.<filename> exists? → file is deleted, return -ENOENT
 * 2. upper_dir/<filename> exists?     → return upper path
 * 3. lower_dir/<filename> exists?     → return lower path
 * 4. Nothing found                    → return -ENOENT
 */
int resolve_path(const char *path, char *resolved_path) {
    struct mini_unionfs_state *state = UNIONFS_DATA;

    // Build whiteout path: upper_dir/.wh.<filename>
    // e.g. path = "/config.txt" → upper/.wh.config.txt
    char whiteout_path[4096];
    char upper_path[4096];
    char lower_path[4096];

    // Extract just the filename from the path for whiteout
    // e.g. "/some/dir/file.txt" → we need "upper/some/dir/.wh.file.txt"
    const char *basename = strrchr(path, '/');
    if (basename == NULL) basename = path;
    // basename now points to the last '/' or the start

    // Build whiteout: insert .wh. before the filename
    char dir_part[4096];
    char file_part[4096];

    // Split path into directory and filename
    const char *last_slash = strrchr(path, '/');
    if (last_slash == path) {
        // File is at root level e.g. "/file.txt"
        strcpy(dir_part, "/");
        strcpy(file_part, last_slash + 1);
    } else if (last_slash == NULL) {
        // No slash at all (shouldn't happen in FUSE, but handle it)
        strcpy(dir_part, "");
        strcpy(file_part, path);
    } else {
        // e.g. "/some/dir/file.txt"
        strncpy(dir_part, path, last_slash - path);
        dir_part[last_slash - path] = '\0';
        strcpy(file_part, last_slash + 1);
    }

    // Special case: path is exactly "/"
    if (strcmp(path, "/") == 0) {
        snprintf(resolved_path, 4096, "%s", state->upper_dir);
        return 0;
    }

    // Build whiteout path
    if (strcmp(dir_part, "/") == 0) {
        snprintf(whiteout_path, sizeof(whiteout_path),
                 "%s/.wh.%s", state->upper_dir, file_part);
    } else {
        snprintf(whiteout_path, sizeof(whiteout_path),
                 "%s%s/.wh.%s", state->upper_dir, dir_part, file_part);
    }

    // Build upper path
    snprintf(upper_path, sizeof(upper_path), "%s%s", state->upper_dir, path);

    // Build lower path
    snprintf(lower_path, sizeof(lower_path), "%s%s", state->lower_dir, path);

    // Step 1: Check whiteout
    if (access(whiteout_path, F_OK) == 0) {
        return -ENOENT;  // File has been "deleted" via whiteout
    }

    // Step 2: Check upper dir
    if (access(upper_path, F_OK) == 0) {
        strcpy(resolved_path, upper_path);
        return 0;
    }

    // Step 3: Check lower dir
    if (access(lower_path, F_OK) == 0) {
        strcpy(resolved_path, lower_path);
        return 0;
    }

    // Step 4: Not found anywhere
    return -ENOENT;
}
