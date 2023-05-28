#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <inttypes.h>

#define BLUE  "\x1b[34m"
#define RESET "\x1b[0m"

typedef struct counter {
    size_t dirs;
    size_t files;
} counter_t;

typedef struct entry {
    char *name;
    int is_dir;
    char permissions[11];
    char user[256];
    char group[256];
    off_t size;
    struct entry *next;
} entry_t;

int walk(const char* directory, const char* prefix, counter_t *counter) {
    entry_t *head = NULL, *current, *iter;
    size_t size = 0, index;

    struct dirent *file_dirent;
    DIR *dir_handle;

    char *full_path, *segment, *pointer, *next_prefix;

    dir_handle = opendir(directory);
    if (!dir_handle) {
        fprintf(stderr, "Cannot open directory \"%s\"\n", directory);
        return -1;
    }

    counter->dirs++;

    while ((file_dirent = readdir(dir_handle)) != NULL) {
        if (file_dirent->d_name[0] == '.') {
            continue;
        }

        current = malloc(sizeof(entry_t));
        current->name = strcpy(malloc(strlen(file_dirent->d_name) + 1), file_dirent->d_name);
        current->is_dir = file_dirent->d_type == DT_DIR;
        struct stat file_stat;
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", directory, file_dirent->d_name);

        if (stat(full_path, &file_stat) == 0) {
            // Permissions
//            printf("1: %s\n",current->permissions);
            sprintf(current->permissions, "%s" ,(S_ISDIR(file_stat.st_mode)) ? "d" : "-");
            sprintf(current->permissions+1, "%s" ,(file_stat.st_mode & S_IRUSR) ? "r" : "-");
            sprintf(current->permissions+2, "%s" ,(file_stat.st_mode & S_IWUSR) ? "w" : "-");
            sprintf(current->permissions+3, "%s" ,(file_stat.st_mode & S_IXUSR) ? "x" : "-");
            sprintf(current->permissions+4, "%s" ,(file_stat.st_mode & S_IRGRP) ? "r" : "-");
            sprintf(current->permissions+5, "%s" ,(file_stat.st_mode & S_IWGRP) ? "w" : "-");
            sprintf(current->permissions+6, "%s" ,(file_stat.st_mode & S_IXGRP) ? "x" : "-");
            sprintf(current->permissions+7, "%s" ,(file_stat.st_mode & S_IROTH) ? "r" : "-");
            sprintf(current->permissions+8, "%s" ,(file_stat.st_mode & S_IWOTH) ? "w" : "-");
            sprintf(current->permissions+9, "%s" ,(file_stat.st_mode & S_IXOTH) ? "x" : "-");
            sprintf(current->permissions+10, "%s" ,"\0");

            // User
            struct passwd *pw = getpwuid(file_stat.st_uid);
            if (pw != NULL) {
                strncpy(current->user, pw->pw_name, sizeof(current->user));
                current->user[sizeof(current->user) - 1] = '\0';
            }

            // Group
            struct group *gr = getgrgid(file_stat.st_gid);
            if (gr != NULL) {
                strncpy(current->group, gr->gr_name, sizeof(current->group));
                current->group[sizeof(current->group) - 1] = '\0';
            }

            // Size
            current->size = file_stat.st_size;
        }

        current->next = NULL;

        if (head == NULL) {
            head = current;
        } else {
            for (iter = head; iter->next; iter = iter->next);
            iter->next = current;
        }

        size++;
    }

    closedir(dir_handle);
    if (!head) {
        return 0;
    }

    for (index = 0; index < size; index++) {
        if (index == size - 1) {
            pointer = "└── ";
            segment = "    ";
        } else {
            pointer = "├── ";
            segment = "│   ";
        }
        if (head->permissions[0]=='d'){
            printf("%s%s[%s %s %s        %ld]  "BLUE"%s\n"RESET, prefix, pointer, head->permissions, head->user, head->group, (intmax_t)head->size, head->name);
        }
        else{
            printf("%s%s[%s %s %s        %ld]  %s\n", prefix, pointer, head->permissions, head->user, head->group, (intmax_t)head->size, head->name);
        }

        if (head->is_dir) {
            full_path = malloc(strlen(directory) + strlen(head->name) + 2);
            sprintf(full_path, "%s/%s", directory, head->name);

            next_prefix = malloc(strlen(prefix) + strlen(segment) + 1);
            sprintf(next_prefix, "%s%s", prefix, segment);

            walk(full_path, next_prefix, counter);
            free(full_path);
            free(next_prefix);
        } else {
            counter->files++;
        }

        current = head;
        head = head->next;

        free(current->name);
        free(current);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    char* directory = argc > 1 ? argv[1] : ".";
    printf(BLUE"%s\n"RESET, directory );

    counter_t counter = {0, 0};
    walk(directory, "", &counter);

    printf("\n%zu directories, %zu files\n",
           counter.dirs ? counter.dirs - 1 : 0, counter.files);
    return 0;
}
