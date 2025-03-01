#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_TOKENS 64

// Global variable to store the search path
char *paths[MAX_TOKENS] = {"/bin", NULL};  // Default search path
int path_count = 1; // Number of paths in the search path


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

// Function to clear existing path
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

void builtInCMD(char command[], char *args[], int count, char outputFile[]){
    //placeholder to ensure arguments and output file is passed properly
    if (outputFile != NULL){
        printf("Output file: %s\n", outputFile);}
    for (int i = 0; i < count; i++) {
        printf("argument %d: %s\n", i + 1, args[i]);
    }

    if (strcmp(command,"exit") == 0) { //Builtin Command Exit
        if(count != 0){
            printf("exit error: more than 0 Arguments\n");
        }
        else{
            free(command);
            exit(0);
        }
        } else if (strcmp(command,"cd") == 0){ //Builtin Command cd
            if(count != 1){ //checks if there is only 1 argument else fail
                printf("cd Error: no arguments or more than 1 argument was passed\n");
                return;
            }
            if(chdir(args[0]) != 0){ //runs chdir() to change directory if 0 isn't returned directory didn't change, print error
            printf("chdir failed \n");
            }
        } else if (strcmp(command, "path") == 0) { 
            updatePath(args, count); //update the path with new directories
        }
}

void executeCMD(char command[], char *args[], int count, char outputFile[]){
    pid_t pid = fork(); //forks a child process
    
    if (pid < 0) {
        printf("Fork failed\n");
        return;
    } else if (pid == 0) { // Child process
        // Handling redirection}
        if (outputFile != NULL) {
            int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                printf("Failed to open output file\n");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO); // Redirect stdout to the file
            dup2(fd, STDERR_FILENO); // Redirect stderr to the file
            close(fd);
        }

        // Search for the command in the path
        char commandPath[256];
        for (int i = 0; i < path_count; i++) {
            snprintf(commandPath, sizeof(commandPath), "%s/%s", paths[i], command);
            if (access(commandPath, X_OK) == 0) { // Check if the command is executable
                execv(commandPath, args); // Execute the command
                // If execv returns, it means there was an error
                printf("execv failed");
                exit(1);
            }
        }
        printf("Command not found: %s\n", command);
        exit(1); // Exit child process
    } else { // Parent process
    wait(NULL); // Wait for child process to finish
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
                printf("Redirection ERROR\n");
                return; //future kill child once parrellel is implemented
            printf("Redirection ERROR\n");
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
                } else{printf("error");break;}
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

void executeCommands(char *commands[], int count){ //this function will execute all the commands that the user inputs
    //currently execute a command one at a time need to change it so it execute parallel
    for (int i = 0; i < count; i++) {
        processCommand(commands[i]);
}}

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

int main() {
    char *line = NULL; //pointer for storing input
    size_t len =0; //Buffer size
    ssize_t read; 


    while(1){
        printf("wish> "); //Print Shell Prompt
        read = getline(&line, &len, stdin);
        if(read == -1){ //Incase shell hits the end of file or user does CTRL+D
            printf("EOF or error detected. Exiting...\n"); //EOF or error detected
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