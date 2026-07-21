#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    char input[1024];
    
    while (1) {
        printf("Picoshell waiting for your command> ");
        
        if (fgets(input, 1024, stdin) == NULL) {
            break; 
        }
        
        input[strcspn(input, "\n")] = 0;
        
        char *my_argv[100]; 
        int my_argc = 0;
        
        char *token = strtok(input, " ");
        
        while (token != NULL) {
            my_argv[my_argc] = token;
            my_argc++;
            token = strtok(NULL, " ");
        }
        
        if (my_argc == 0) {
            continue;
        }

        if (strcmp(my_argv[0], "echo") == 0) {
            for (int i = 1; i < my_argc; i++) {
                if (i == (my_argc - 1)) {
                    printf("%s", my_argv[i]);   
                } else {
                    printf("%s ", my_argv[i]);
                }
            }
            printf("\n");
        }
        
        else if (strcmp(my_argv[0], "exit") == 0) {
            printf("Good Bye\n");
            break;
        }
        
        else if (strcmp(my_argv[0], "pwd") == 0) {
            char my_array[4096];    
            char *x = getcwd(my_array, 4096);
            if (x == NULL) {
                printf("Error pwd failed\n");
            } else {
                printf("%s\n", my_array);
            }
        }
        
        else if (strcmp(my_argv[0], "cd") == 0) {
            if (my_argc < 2) {
                printf("error invalid number of arguments\n");
            } 
            else if (chdir(my_argv[1]) != 0) {
                printf("error\n");
            }
        }
        
        else {
            printf("Invalid command\n");
        }
    }
    
    return 0;
}
