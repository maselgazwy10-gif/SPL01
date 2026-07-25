#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    char input[65536];
    int exit_status = 0; 
    
    while (1) {
        printf("Picoshell waiting for your command> ");
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; 
        }
        
        input[strcspn(input, "\n")] = 0;
        
        char *my_argv[2048]; 
        int my_argc = 0;
        
        char *token = strtok(input, " ");
        
        while (token != NULL && my_argc < 2047) {
            my_argv[my_argc] = token;
            my_argc++;
            token = strtok(NULL, " ");
        }
        my_argv[my_argc] = NULL; 
        
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
            exit_status = 0;
        }
        
        else if (strcmp(my_argv[0], "exit") == 0) {
            printf("Good Bye\n");
            break;
        }
        
        else if (strcmp(my_argv[0], "pwd") == 0) {
            char my_array[65536]; 
            char *x = getcwd(my_array, sizeof(my_array));
            if (x == NULL) {
                printf("Error pwd failed\n");
                exit_status = 1;
            } else {
                printf("%s\n", my_array);
                exit_status = 0;
            }
        }
        
        else if (strcmp(my_argv[0], "cd") == 0) {
            if (my_argc < 2) {
                printf("cd: missing argument\n");
                exit_status = 1;
            } 
            else if (chdir(my_argv[1]) != 0) {
                printf("cd: %s: No such file or directory\n", my_argv[1]);
                exit_status = 1;
            } else {
                exit_status = 0;
            }
        }
        
        else {
            pid_t pid = fork();
            if (pid < 0) {
                perror("Fork failed");
                exit_status = 1;
            } else if (pid == 0) {
                execvp(my_argv[0], my_argv);
                
                printf("%s: command not found\n", my_argv[0]);
                exit(127); 
            } else {
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status)) {
                    exit_status = WEXITSTATUS(status);
                }
            }
        }
    }
    
    return exit_status;
}
