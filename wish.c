#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 64

void executeCommands(char *commands[], int count){
    //currently prints commands change to Actually execute the commands
    for (int i = 0; i < count; i++) {
    printf("Execute Command %d: %s\n", i + 1, commands[i]);
    }
}

void parseInput(char line[]){
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

        if (line[read - 1] == '\n') { //remove newline character from input
            line[read - 1] = '\0';
        }

        if (strcmp(line,"exit") == 0) { //if user types exit, exit shell
            free(line);
            exit(0);
        }

        parseInput(line);
    }



    return 0;
}