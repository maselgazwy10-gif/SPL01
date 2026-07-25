#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

char **var_names = NULL;
char **var_values = NULL;
int var_count = 0;
int var_capacity = 0;

const char* get_var(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(var_names[i], name) == 0) {
            return var_values[i];
        }
    }
    
    char *env_val = getenv(name);
    
    if (env_val != NULL) {
        return env_val;
    } else {
        return "";
    }
}

void set_var(const char *name, const char *value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(var_names[i], name) == 0) {
            free(var_values[i]); 
            var_values[i] = strdup(value);
            return;
        }
    }
    
    if (var_count >= var_capacity) {
        if (var_capacity == 0) {
            var_capacity = 10;
        } else {
            var_capacity = var_capacity * 2;
        }
        var_names = realloc(var_names, var_capacity * sizeof(char*));
        var_values = realloc(var_values, var_capacity * sizeof(char*));
    }
    
    var_names[var_count] = strdup(name);
    var_values[var_count] = strdup(value);
    var_count++;
}

void cleanup_vars() {
    for (int i = 0; i < var_count; i++) {
        free(var_names[i]);
        free(var_values[i]);
    }
    free(var_names);
    free(var_values);
}

int main() {
    char input[65536];
    int exit_status = 0; 
    
    while (1) {
        printf("NanoShell> ");
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; 
        }
        
        input[strcspn(input, "\n")] = 0;
        
        char *my_argv[2048]; 
        int my_argc = 0;
        
        char *token = strtok(input, " ");
        
        while (token != NULL && my_argc < 2047) {
            char *dollar = strchr(token, '$');
            
            if (dollar != NULL) {
                *dollar = '\0'; 
                char *var_name = dollar + 1;
                const char *val = get_var(var_name);
                
                char *expanded = malloc(strlen(token) + strlen(val) + 1);
                sprintf(expanded, "%s%s", token, val);
                my_argv[my_argc] = expanded;
            } else {
                my_argv[my_argc] = strdup(token);
            }
            
            my_argc++;
            token = strtok(NULL, " ");
        }
        my_argv[my_argc] = NULL; 
        
        if (my_argc == 0) {
            continue;
        }

        if (strchr(my_argv[0], '=') != NULL) {
            if (my_argc != 1 || my_argv[0][0] == '=' || my_argv[0][strlen(my_argv[0]) - 1] == '=') {
                printf("Error: Invalid variable assignment format\n");
                exit_status = 1;
            } else {
                char *eq_pos = strchr(my_argv[0], '=');
                *eq_pos = '\0'; 
                set_var(my_argv[0], eq_pos + 1);
                exit_status = 0;
            }
        }
        
        else if (strcmp(my_argv[0], "export") == 0) {
            if (my_argc < 2) {
                printf("export: missing argument\n");
                exit_status = 1;
            } else {
                const char *val = get_var(my_argv[1]);
                if (val[0] != '\0') {
                    setenv(my_argv[1], val, 1); 
                }
                exit_status = 0;
            }
        }
        
        else if (strcmp(my_argv[0], "echo") == 0) {
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
            for (int i = 0; i < my_argc; i++) free(my_argv[i]);
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
        
        for (int i = 0; i < my_argc; i++) {
            free(my_argv[i]);
        }
    }
    
    cleanup_vars();
    return exit_status;
}
