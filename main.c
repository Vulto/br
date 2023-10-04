#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_FILES 1024

int is_current_or_parent_dir(const char *dir) {
    return (strcmp(dir, ".") == 0 || strcmp(dir, "..") == 0);
}

int main(int argc, char *argv[]) {
    char filenames_file[] = "/tmp/vimv.XXXXXX";
    char **src = NULL;
    char **dest = NULL;
    int count = 0;

    // Initialize src array
    if (argc != 1) {
        // Use command-line arguments as src files/directories
        src = malloc((argc - 1) * sizeof(char *));
        for (int i = 0; i < argc - 1; i++) {
            src[i] = strdup(argv[i + 1]);
        }
    } else {
        // Read files/directories from current directory
        src = malloc(MAX_FILES * sizeof(char *));
        size_t i = 0;
        char path[PATH_MAX];
        getcwd(path, PATH_MAX);
        DIR *dir = opendir(path);
        if (dir == NULL) {
            perror("Cannot open directory");
            exit(EXIT_FAILURE);
        }
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL && i < MAX_FILES) {
            if (is_current_or_parent_dir(entry->d_name)) {
                continue;
            }
            struct stat file_stat;
            if (stat(entry->d_name, &file_stat) == -1) {
                perror("Failed to get file status");
                continue;
            }
            if (S_ISREG(file_stat.st_mode) || S_ISDIR(file_stat.st_mode)) { // Regular file or directory
                src[i++] = strdup(entry->d_name);
            }
        }
        closedir(dir);
    }

    // Create unique filenames file
    int fd = mkstemp(filenames_file);
    if (fd == -1) {
        perror("Cannot create unique filenames file");
        exit(EXIT_FAILURE);
    }
    close(fd);

    // Write filenames to tempfile
    FILE *fileptr = fopen(filenames_file, "w");
    if (fileptr == NULL) {
        perror("Cannot open temporary file");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; src[i] != NULL; i++) {
        fprintf(fileptr, "%s\n", src[i]);
    }
    fclose(fileptr);

    // Open tempfile in editor
    char *editor = getenv("EDITOR");
    if (editor == NULL) {
        editor = "vim";
    }
    char command[PATH_MAX];
    snprintf(command, PATH_MAX, "%s %s", editor, filenames_file);
    if (system(command) != 0) {
        perror("Error while opening editor");
        exit(EXIT_FAILURE);
    }

    // Read filenames from tempfile
    fileptr = fopen(filenames_file, "r");
    if (fileptr == NULL) {
        perror("Cannot open temporary file");
        exit(EXIT_FAILURE);
    }
    dest = malloc(MAX_FILES * sizeof(char *));
    size_t i = 0;
    char line[PATH_MAX];
    while (fgets(line, PATH_MAX, fileptr) != NULL && i < MAX_FILES) {
        line[strcspn(line, "\n")] = '\0'; // Remove trailing newline
        dest[i] = malloc(strlen(line) + 1); // Allocate memory for renamed filename
        strcpy(dest[i++], line);
    }
    fclose(fileptr);
    dest[i] = NULL;

    // Check number of files/directories and rename them
    for (size_t i = 0; src[i] != NULL && dest[i] != NULL; i++) {
        if (strcmp(src[i], dest[i]) != 0) {
            struct stat st;
            if (stat(dest[i], &st) == 0 && (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))) {
                // Destination exists as a regular file or directory
                printf("%s already exists. Skipping.\n", dest[i]);
                continue;
            }
            if (rename(src[i], dest[i]) == -1) {
                perror("Error while renaming file/directory");
                exit(EXIT_FAILURE);
            } else {
                count++;
            }
        }
    }

    printf("%d files/directories renamed.\n", count);

    // Free memory
    for (size_t i = 0; src[i] != NULL; i++) {
        free(src[i]);
    }
    free(src);
    for (size_t i = 0; dest[i] != NULL; i++) {
        free(dest[i]);
    }
    free(dest);

    return EXIT_SUCCESS;
}
