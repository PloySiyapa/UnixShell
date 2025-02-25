
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

        printf("you entered: %s \n", line);
    }



    return 0;
}