#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS 64

void removeWhitespace(char *str) {
    int start = 0, end = strlen(str) - 1;
    while (isspace((unsigned char)str[start])) start++;
    while (end > start && isspace((unsigned char)str[end])) end--;
    int i;
    for (i = 0; i <= end - start; i++) str[i] = str[start + i];
    str[i] = '\0';
}

void executeCommand(char *command) {
    if (strcmp(command, "exit") == 0) {
        printf("Executing command: exit\n");
        exit(0);
    }
    
    printf("Executing command: %s\n", command);
    system(command);
}

void processFileInput(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        exit(1);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        removeWhitespace(line);
        if (line[0] == '\0') continue;  // Skip empty lines

        printf("wish> %s\n", line);  // Simulate command input
        executeCommand(line);
    }

    free(line);
    fclose(file);
}

void interactiveMode() {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while (1) {
        printf("wish> ");
        read = getline(&line, &len, stdin);
        if (read == -1) {
            printf("\nExiting...\n");
            free(line);
            exit(0);
        }

        removeWhitespace(line);
        if (line[0] == '\0') continue;  // Skip empty lines

        executeCommand(line);
    }
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        processFileInput(argv[1]);  // Run commands from batch file
    } else {
        interactiveMode();  // Run shell interactively
    }

    return 0;
}

