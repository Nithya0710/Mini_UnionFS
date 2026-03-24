#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Standalone test for resolve_path logic.
 * We simulate upper/lower dirs using real temp directories.
 * Tests all 4 cases:
 *   1. Whiteout exists       → should return -ENOENT
 *   2. File in upper only    → should return upper path
 *   3. File in lower only    → should return lower path
 *   4. File nowhere          → should return -ENOENT
 *   5. File in both          → should return upper path (upper wins)
 *   6. Path is "/"           → should return upper_dir root
 */

// Replicate state and resolve_path here for standalone testing
// (without needing FUSE context)

#define MAX_PATH 4096

char g_upper[MAX_PATH];
char g_lower[MAX_PATH];

int resolve_path_test(const char *path, char *resolved_path) {
    char whiteout_path[MAX_PATH];
    char upper_path[MAX_PATH];
    char lower_path[MAX_PATH];
    char dir_part[MAX_PATH];
    char file_part[MAX_PATH];

    if (strcmp(path, "/") == 0) {
        snprintf(resolved_path, MAX_PATH, "%s", g_upper);
        return 0;
    }

    const char *last_slash = strrchr(path, '/');
    if (last_slash == path) {
        strcpy(dir_part, "/");
        strcpy(file_part, last_slash + 1);
    } else if (last_slash == NULL) {
        strcpy(dir_part, "");
        strcpy(file_part, path);
    } else {
        strncpy(dir_part, path, last_slash - path);
        dir_part[last_slash - path] = '\0';
        strcpy(file_part, last_slash + 1);
    }

    if (strcmp(dir_part, "/") == 0) {
        snprintf(whiteout_path, MAX_PATH, "%s/.wh.%s", g_upper, file_part);
    } else {
        snprintf(whiteout_path, MAX_PATH, "%s%s/.wh.%s", g_upper, dir_part, file_part);
    }

    snprintf(upper_path, MAX_PATH, "%s%s", g_upper, path);
    snprintf(lower_path, MAX_PATH, "%s%s", g_lower, path);

    if (access(whiteout_path, F_OK) == 0) return -1; // ENOENT
    if (access(upper_path,    F_OK) == 0) { strcpy(resolved_path, upper_path); return 0; }
    if (access(lower_path,    F_OK) == 0) { strcpy(resolved_path, lower_path); return 0; }
    return -1; // ENOENT
}

void make_file(const char *path) {
    FILE *f = fopen(path, "w");
    if (f) { fprintf(f, "test"); fclose(f); }
}

int passed = 0, failed = 0;

void check(const char *name, int got, int expected_ret, const char *got_path, const char *expected_path) {
    int ret_ok = (got == expected_ret);
    int path_ok = (expected_path == NULL) || (got_path && strcmp(got_path, expected_path) == 0);
    if (ret_ok && path_ok) {
        printf("  ✅ PASS: %s\n", name);
        passed++;
    } else {
        printf("  ❌ FAIL: %s\n", name);
        printf("     Expected ret=%d path=%s\n", expected_ret, expected_path ? expected_path : "N/A");
        printf("     Got     ret=%d path=%s\n", got, got_path ? got_path : "NULL");
        failed++;
    }
}

int main() {
    // Setup temp dirs
    strcpy(g_upper, "/tmp/unionfs_test_upper");
    strcpy(g_lower, "/tmp/unionfs_test_lower");
    system("rm -rf /tmp/unionfs_test_upper /tmp/unionfs_test_lower");
    mkdir(g_upper, 0755);
    mkdir(g_lower, 0755);

    char resolved[MAX_PATH];
    int ret;

    printf("\n=== resolve_path() Unit Tests ===\n\n");

    // Test 1: File only in lower
    make_file("/tmp/unionfs_test_lower/lower_only.txt");
    ret = resolve_path_test("/lower_only.txt", resolved);
    check("File only in lower → returns lower path",
          ret, 0, resolved, "/tmp/unionfs_test_lower/lower_only.txt");

    // Test 2: File only in upper
    make_file("/tmp/unionfs_test_upper/upper_only.txt");
    ret = resolve_path_test("/upper_only.txt", resolved);
    check("File only in upper → returns upper path",
          ret, 0, resolved, "/tmp/unionfs_test_upper/upper_only.txt");

    // Test 3: File in both → upper wins
    make_file("/tmp/unionfs_test_lower/both.txt");
    make_file("/tmp/unionfs_test_upper/both.txt");
    ret = resolve_path_test("/both.txt", resolved);
    check("File in both → upper wins",
          ret, 0, resolved, "/tmp/unionfs_test_upper/both.txt");

    // Test 4: Whiteout exists → ENOENT
    make_file("/tmp/unionfs_test_lower/deleted.txt");
    make_file("/tmp/unionfs_test_upper/.wh.deleted.txt");
    ret = resolve_path_test("/deleted.txt", resolved);
    check("Whiteout exists → returns -ENOENT",
          ret, -1, NULL, NULL);

    // Test 5: File nowhere → ENOENT
    ret = resolve_path_test("/ghost.txt", resolved);
    check("File nowhere → returns -ENOENT",
          ret, -1, NULL, NULL);

    // Test 6: Path is "/"
    ret = resolve_path_test("/", resolved);
    check("Path is '/' → returns upper_dir",
          ret, 0, resolved, g_upper);

    printf("\nResults: %d passed, %d failed\n\n", passed, failed);

    // Cleanup
    system("rm -rf /tmp/unionfs_test_upper /tmp/unionfs_test_lower");
    return failed > 0 ? 1 : 0;
}
