#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include "nmenu.h"

void rename_file(char* oldname, char* newname) {
    int result = rename(oldname, newname);
    if (result != 0) {
        printf("Error renaming files %s\n", oldname);
    }else{
        printf("%s Files renamed\n", oldname);
	}
}

#define D_SIZE 10

typedef struct {
    char **strs;
    int size;
    int used;
} darr;

void darr_push(darr *arr, char *str) {
    if (arr->used == arr->size) {
        arr->size *= 2;
        arr->strs = realloc(arr->strs, arr->size * sizeof(char *));
    }
    arr->strs[arr->used++] = strdup(str);
}

void darr_free(darr *arr) {
    for (int i = 0; i < arr->used; i++) {
        free(arr->strs[i]);
    }
    free(arr->strs);
}

int main(void) {
    DIR *dir;
    struct dirent *entry;
    dir = opendir(".");
    if (dir == NULL) {
        printf("Error opening directory.\n");
        exit(1);
    }

    darr options = {NULL, D_SIZE, 0};
    options.strs = malloc(options.size * sizeof(char *));

    while ((entry = readdir(dir)) != NULL) {
        char *filename = entry->d_name;
        if (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0) {
            darr_push(&options, filename);
        }
    }

    // Prompt user to select files to rename
    int idx = nmenu(options.strs, options.used, "Select files to rename:");

    // Get new name for selected file
    char *oldname = options.strs[idx];
    printf("Enter new name for file %s: ", oldname);
    char newname[MAX_STR_SIZE];
    fgets(newname, MAX_STR_SIZE, stdin);
    newname[strcspn(newname, "\n")] = 0; // remove trailing newline from input

    rename_file(oldname, newname);

    darr_free(&options);
    closedir(dir);
    return EXIT_SUCCESS;
}
