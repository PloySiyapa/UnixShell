#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>  // Add for waitpid
#include <fcntl.h>     // Add for file operations

#define MAX_TOKENS 64

// Global variable to store the search path
char *paths[MAX_TOKENS];  // Default search path
int path_count = 0; // Number of paths in the search path

void updatePath(char *args[], int count) {
    // Clear the existing path
    for (int i = 0; i < path_count; i++) {
        free(paths[i]);
    }

    path_count = 0; // Reset path count

    // Add new directories to the path
    for (int i = 0; i < count; i ++) {
        paths[i] = strdup(args[i]); // store the new path
        path_count++;
    }
}

void removeWhitespace(char *str) { //this function removes whitespaces to make string comparison more consistent
    int start = 0;
    int end = strlen(str) - 1;

    while (isspace((unsigned char)str[start])) { //remove the whitespaces if any at the beginning of the word 
        start++;
    }

    while (end > start && isspace((unsigned char)str[end])) { //removes the whitespaces if any at the end of the word
        end--;
    }

    int i;
    for (i = 0; i <= end - start; i++) { //shifts characters in the array
        str[i] = str[start + i];
    }
    str[i] = '\0'; //adds null-terminate to the end of the char array
}

void builtInCMD(char command[], char *args[], int count, char outputFile[]){
    const char *exit_error_message = "exit ERROR: Cannot have any arguments\n";
    const char *cd_error_message = "cd ERROR: Must have only one argument\n";
    const char *chdir_error_message = "chdir ERROR: chdir failed\n";
    
    if (strcmp(command,"exit") == 0) { //Builtin Command Exit
        if(count != 0){
            write(STDERR_FILENO, exit_error_message, strlen(exit_error_message));
        }
        else{
            exit(0);
        }
    }
    else if (strcmp(command,"cd")==0){ //Builtin Command cd
        if(count != 1){ //checks if there is only 1 argument else fail
            write(STDERR_FILENO, cd_error_message, strlen(cd_error_message));
            return;
        }
        if(chdir(args[0]) != 0){ //runs chdir() to change directory
            write(STDERR_FILENO, chdir_error_message, strlen(chdir_error_message));
        }
    }
    else if (strcmp(command,"path")==0){ 
        updatePath(args, count); //update the path with new directories
    }
}

void executeCMD(char command[], char *args[], int count, char outputFile[]){
    char commandPath[256];
    int found = 0;
    
    // Search for command in paths
    for (int i = 0; i < path_count; i++) {
        snprintf(commandPath, sizeof(commandPath), "%s/%s", paths[i], command);
        if (access(commandPath, X_OK) == 0) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    
    // Handle redirection if specified
    if (outputFile != NULL) {
        int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            return;
        }
        dup2(fd, STDOUT_FILENO); // Redirect stdout
        dup2(fd, STDERR_FILENO); // Also redirect stderr as per spec
        close(fd);
    }
    
    // Prepare arguments for execv
    char *execArgs[count + 2];
    execArgs[0] = commandPath;  // Use full path instead of just command
    for (int j = 0; j < count; j++) {
        execArgs[j + 1] = args[j];
    }
    execArgs[count + 1] = NULL;  // NULL terminate the args array
    
    // Execute the command - no need to fork as we're already in a child process
    if(execv(commandPath, execArgs) == -1){
        // If execv returns, it failed
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
}

void processCommand(char *commandString){
    char *args[MAX_TOKENS]; //array will store arguments
    char *token; //pointer to store each argmuent
    char *rest = commandString; //tracks the remainder of string
    int arg_count = 0; //will count the number of arguments found
    char *redirect = commandString; //will hold redirection output file
    char *command;

    if(strsep(&redirect, ">") != NULL){
       if(redirect != NULL){ //redirection error detection
           removeWhitespace(redirect);
           //checks if > operator is called more than once and if there is multiple output files, if so raise error
           if (strchr(redirect, ' ') != NULL || strchr(redirect, '>') != NULL) {
                const char *redirection_error_message = "Error: Multiple redirection operators or multiple output files to the right of redirection sign\n";
                write(STDERR_FILENO, redirection_error_message, strlen(redirection_error_message));
                return; //future kill child once parrellel is implemented
            }
       }
    }
    removeWhitespace(rest);
    //checks if there are arguments indicated by " ", if true store first portion of strsep to command, and the rest of the lines would be arguments
    if((command = strsep(&rest, " ")) != NULL){ 
        //splits arguments line by " " and stores it in args array
        if(rest != NULL){
            removeWhitespace(rest);
            while((token = strsep(&rest, " ")) != NULL){
                if (*token == '\0')  // Skip empty tokens caused by multiple spaces
                continue;

                if(arg_count < MAX_TOKENS){
                    args[arg_count++] = token;
                }
                else{printf("error");break;}
                }
            }}
    else
        command=rest; //if no argumemts detected set command to the rest of the line
    
    removeWhitespace(command);
    //this will compare the command varible as see if it is a builtin command, if so call builtInCMD function, else call executeCMD function
    if(strcmp(command,"exit") == 0|| strcmp(command,"path") == 0 || strcmp(command,"cd") == 0){
        builtInCMD(command, args, arg_count, redirect);}
    else{
        executeCMD(command, args, arg_count, redirect);}
}

void executeCommands(char *commands[], int count){
    pid_t pids[count];
    int child_count = 0;
    
    // Process each command
    for (int i = 0; i < count; i++) {
        char *cmd_copy = strdup(commands[i]);
        if (!cmd_copy) {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            continue;
        }
        
        // Extract the command name to check if it's a built-in
        char *rest = cmd_copy;
        char *redirect_check = strdup(cmd_copy);
        char *command = NULL;
        
        // Handle potential redirection in the command string
        strsep(&redirect_check, ">");
        
        // Extract the command name
        removeWhitespace(rest);
        command = strsep(&rest, " ");
        
        if (command) {
            removeWhitespace(command);
            
            // Check if it's a built-in command
            if (strcmp(command, "exit") == 0 || 
                strcmp(command, "cd") == 0 || 
                strcmp(command, "path") == 0) {
                // Execute built-in command in the parent process
                processCommand(commands[i]);
                free(cmd_copy);
                free(redirect_check);
                continue;
            }
        }
        free(cmd_copy);
        //free(redirect_check);
        
        // For non-built-in commands, fork and execute
        pids[child_count] = fork();
        
        if (pids[child_count] < 0) {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            continue;
        }
        
        if (pids[child_count] == 0) {  // Child process
            processCommand(commands[i]);
            exit(0);  // Exit after processing command
        }
        
        child_count++;
    }
    
    // Parent waits for all children to finish
    for (int i = 0; i < child_count; i++) {
        if (pids[i] > 0) {
            waitpid(pids[i], NULL, 0);
        }
    }
}

void splitInput(char line[]){
    char *commands[MAX_TOKENS]; //array that will store commands
    char *token; //pointer that sores each command that has been extracted
    char *rest = line; //pointer that till track the remaining part of string to be parsed
    int count = 0; //counter to track numer of stored commands

    //strsep() splits the string in half if it finds "&"
    while((token = strsep(&rest, "&")) != NULL){
        if(count < MAX_TOKENS){ 
            commands[count++] = token; //stores the extracted command in the array
        }
        else{ //If stop parsing if maximum number of commands is reached
            break;
        }

    }

    executeCommands(commands, count);
}

int main(int argc, char *argv[]) {

    paths[0] = strdup("/bin"); //initial shell path should contain one directory
    path_count = 1;

    char *line = NULL; //pointer for storing input
    size_t len =0; //Buffer size
    ssize_t read; 
    FILE *input = stdin; // Default shell input to interactive mode instead of batch

    // Check if running in batch mode
    if (argc == 2) {
        input = fopen(argv[1], "r");  // Open batch file
        if (!input) {
            const char *open_error_message = "Error opening file\n";
            write(STDERR_FILENO, open_error_message, strlen(open_error_message));
            exit(1);
        }
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
        exit(1);
    }

    while(1){
        printf("wish> "); //Print Shell Prompt
        fflush(stdout);

        read = getline(&line, &len, input);
        if(read == -1){ //Incase shell hits the end of file or user does CTRL+D
            free(line); //free memory before exiting
            exit(0);
        }
        
        removeWhitespace(line);
        if(line[0] == '\0'){continue;} //skip rest of loop because user didn't input anything
        
        if (line[read - 1] == '\n') { //remove newline character from input
            line[read - 1] = '\0';
        }
        splitInput(line);
    }

    return 0;
}