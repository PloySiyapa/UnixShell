// Author: Siyapa(Ploy) Chanhorm
// Date: 2/23/2025
// Purpose: Built-in Commands (exit, cd, path)
//          - implement the exit command (should terminate the shell)
//          - implement the cd command with proper error handling
//          - implement the path command to manage directirues for command search
// Note: must run in UNIX system

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT 1024
#define MAX_TOKENS 64
#define MAX_PATHS 64

char *paths[MAX_PATHS] = {"/bin", NULL};  // Default search path

void execute_command(char **args, int redirect, char *outfile) {
    if (args[0] == NULL) return;
    
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL || chdir(args[1]) != 0) {
            fprintf(stderr, "wish: cd failed\n");
        }
        return;
    } else if (strcmp(args[0], "path") == 0) {
        int i = 0;
        memset(paths, 0, sizeof(paths));
        for (i = 1; args[i] != NULL; i++) {
            paths[i - 1] = strdup(args[i]);
        }
        paths[i - 1] = NULL;
        return;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        if (redirect) {
            int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                fprintf(stderr, "wish: cannot open file\n");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        for (int i = 0; paths[i] != NULL; i++) {
            char cmd[MAX_INPUT];
            snprintf(cmd, sizeof(cmd), "%s/%s", paths[i], args[0]);
            execv(cmd, args);
        }
        fprintf(stderr, "wish: command not found\n");
        exit(1);
    } else {
        wait(NULL);
    }
}

void parse_and_execute(char *input) {
    char *commands[MAX_TOKENS];
    char *token = strtok(input, "&");
    int cmd_count = 0;
    while (token != NULL) {
        commands[cmd_count++] = token;
        token = strtok(NULL, "&");
    }
    
    for (int i = 0; i < cmd_count; i++) {
        char *args[MAX_TOKENS];
        char *outfile = NULL;
        int redirect = 0;
        
        char *cmd = strtok(commands[i], " 	\n");
        int arg_count = 0;
        while (cmd != NULL) {
            if (strcmp(cmd, ">") == 0) {
                cmd = strtok(NULL, " 	\n");
                if (cmd == NULL) {
                    fprintf(stderr, "wish: missing output file\n");
                    return;
                }
                outfile = cmd;
                redirect = 1;
                break;
            }
            args[arg_count++] = cmd;
            cmd = strtok(NULL, " 	\n");
        }
        args[arg_count] = NULL;
        execute_command(args, redirect, outfile);
    }
}

int main() {
    char input[MAX_INPUT];
    while (1) {
        printf("wish> ");
        if (!fgets(input, MAX_INPUT, stdin)) {
            break;
        }
        parse_and_execute(input);
    }
    return 0;
}

