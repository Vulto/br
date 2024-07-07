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

/*
	Nice to have Features like
	edit owner, permission, deletion, move
*/

int is_current_or_parent_dir(const char *dir) {
    return (strcmp(dir, ".") == 0 || strcmp(dir, "..") == 0);
}

void read_from_pipe_or_here_string(char ***src, int *count) {
    char path[PATH_MAX];

    if (isatty(fileno(stdin))) {
        // Read files/directories from current directory
        *src = malloc(MAX_FILES * sizeof(char *));
        *count = 0;

        getcwd(path, PATH_MAX);
        DIR *dir = opendir(path);
        if (dir == NULL) {
            perror("Cannot open directory");
            exit(EXIT_FAILURE);
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL && *count < MAX_FILES) {
            if (is_current_or_parent_dir(entry->d_name)) {
                continue;
            }
            struct stat file_stat;
            if (stat(entry->d_name, &file_stat) == -1) {
                perror("Failed to get file status");
                continue;
            }
            if (S_ISREG(file_stat.st_mode) || S_ISDIR(file_stat.st_mode)) {
                (*src)[(*count)++] = strdup(entry->d_name);
            }
        }

        closedir(dir);
    } else {
        // Read input from pipe or here-string
        *src = malloc(MAX_FILES * sizeof(char *));
        *count = 0;

        char line[PATH_MAX];
        while (fgets(line, PATH_MAX, stdin) != NULL && *count < MAX_FILES) {
            line[strcspn(line, "\n")] = '\0'; // Remove trailing newline
            (*src)[(*count)++] = strdup(line);
        }
    }
}

int main() {
    int count;
    char **src = NULL;
    char **dest = NULL;

    // Read input from pipe or here-string, or current directory
    read_from_pipe_or_here_string(&src, &count);

    // Create unique filenames file
    char filenames_file[] = "/tmp/br.XXXXXX";
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
    for (size_t i = 0; i < count; i++) {
        fprintf(fileptr, "%s\n", src[i]);
    }
    fclose(fileptr);

    // Open tempfile in editor
    char *editor = getenv("EDITOR");
    if (editor == NULL) {
        editor = "nvim";
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
    int rename_count = 0;
    for (size_t i = 0; i < count && dest[i] != NULL; i++) {
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
                rename_count++;
            }
        }
    }

    printf("%d files/directories renamed.\n", rename_count);

    // Free memory
    for (size_t i = 0; i < count; i++) {
        free(src[i]);
    }
    free(src);
    for (size_t i = 0; dest[i] != NULL; i++) {
        free(dest[i]);
    }
    free(dest);

    // Remove temporary filenames file
    remove(filenames_file);

    return EXIT_SUCCESS;
}
