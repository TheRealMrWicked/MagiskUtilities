#include <stdio.h>  // Input/Output Stream
#include <stdbool.h> // Adds the boolean data type
#include <stdlib.h> // Running system commands
#include <sys/stat.h> // Folder manipulation
#include <dirent.h> // Reading Directory's

char* os() {
#ifdef _WIN32
    return "\\";
#elif __linux__
    return "/";
#endif
}

char* commandoutput(char command[]) {
    char fullcommand[60];
    sprintf(fullcommand, "%s > temp.txt",command);
    system(fullcommand);
    FILE *input;
    char *str = malloc(3);

    input = fopen("temp.txt" , "r");

    fgets (str, 10, input);
    fclose(input);
    str[strcspn(str, "\n")] = 0;
    remove("temp.txt");
    return str;
}

// File Manipulation

bool filexists(char filename[]) {
    FILE *file = fopen(filename, "r");
    bool exists = false;

    if (file != NULL) {
        exists = true;
        fclose(file);
    }
    return exists;
}

void wildcardrename(char oldname[], char newname[]) {
    char command[40];

    if (strcmp(os(), "\\") == 0) {
        strncpy(command, "move ", 40);
    } else if (strcmp(os(), "/") == 0) {
        strncpy(command, "mv ", 40);
    }

    strcat(command, oldname);
    strcat(command, " ");
    strcat(command, newname);
    system(command);
}

char* getextension(char filename[]){
    char* extension;
    extension = strrchr(filename, '.');
    if (!extension) {
        return NULL;
    } else {
        return extension + 1;
    }
}

char* removefileextension(char filename[]) {
    char *retStr;
    char *lastExt;

    if (filename == NULL)
        return NULL;

    if ((retStr = malloc (strlen (filename) + 1)) == NULL)
        return NULL;

    strcpy (retStr, filename);
    lastExt = strrchr (retStr, '.');

    if (lastExt != NULL)
        *lastExt = '\0';
    return retStr;
}

// Folder Manipulation

void removefolder(char path[]) {
    size_t pathlength;
    DIR *dir;
    char *fullpath;
    struct stat statpath, statentry;
    struct dirent *entry;

    stat(path, &statpath);

    S_ISDIR(statpath.st_mode);

    dir = opendir(path);

    pathlength = strlen(path);

    while ((entry = readdir(dir)) != NULL) {

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        fullpath = calloc(pathlength + 1 + strlen(entry->d_name) + 1, sizeof(char));
        strcpy(fullpath, path);
        strcat(fullpath, "/");
        strcat(fullpath, entry->d_name);

        stat(fullpath, &statentry);

        if (S_ISDIR(statentry.st_mode) != 0) {
            removefolder(fullpath);
            free(fullpath);
            continue;
        }

        if (unlink(fullpath) == 0)
            free(fullpath);
    }

    if (rmdir(path) == 0)
        closedir(dir);
}

void copyfolder(char srcfolder[], char dstfolder[]) {
    char command[40];
    if (strcmp(os(), "\\") == 0) {
        strncpy(command, "xcopy /E /H /C /I ", 40);
    } else if (strcmp(os(), "/") == 0) {
        strncpy(command, "cp ", 40);
    }

    strcat(command, srcfolder);
    strcat(command, " ");
    strcat(command, dstfolder);
    system(command);
}

