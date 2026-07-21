#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

typedef struct {
    char *name;
    char *value;
} ShellVar;

ShellVar *local_vars = NULL;
int var_count = 0;
int var_capacity = 0;

char* get_any_var(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(local_vars[i].name, name) == 0) {
            return local_vars[i].value;
        }
    }
    char *env_val = getenv(name);
    return (env_val != NULL) ? env_val : "";
}

void set_local_var(const char *name, const char *value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(local_vars[i].name, name) == 0) {
            free(local_vars[i].value);
            local_vars[i].value = strdup(value);
            return;
        }
    }
    if (var_count >= var_capacity) {
        var_capacity = (var_capacity == 0) ? 10 : var_capacity * 2;
        local_vars = realloc(local_vars, var_capacity * sizeof(ShellVar));
    }
    local_vars[var_count].name = strdup(name);
    local_vars[var_count].value = strdup(value);
    var_count++;
}

void free_local_vars() {
    for (int i = 0; i < var_count; i++) {
        free(local_vars[i].name);
        free(local_vars[i].value);
    }
    free(local_vars);
}

int main() {
    char input[1024];
    
    while (1) {
        printf("Nano Shell Prompt > ");
        
        if (fgets(input, 1024, stdin) == NULL) {
            break; 
        }
        
        input[strcspn(input, "\n")] = 0;
        
        int capacity = 10;
        char **my_argv = malloc(capacity * sizeof(char*)); 
        int my_argc = 0;
        
        char *token = strtok(input, " ");
        while (token != NULL) {
            my_argv[my_argc] = token;
            my_argc++;
            
            if (my_argc >= capacity) {
                capacity *= 2;
                my_argv = realloc(my_argv, capacity * sizeof(char*));
            }
            token = strtok(NULL, " ");
        }
        
        if (my_argc == 0) {
            free(my_argv);
            continue;
        }
        my_argv[my_argc] = NULL; 

        int is_malloced[100] = {0}; 
        for (int i = 0; i < my_argc; i++) {
            char *dollar_pos = strchr(my_argv[i], '$');
            if (dollar_pos != NULL) {
                *dollar_pos = '\0'; 
                char *prefix = my_argv[i];        
                char *var_name = dollar_pos + 1;  
                char *var_value = get_any_var(var_name);
                
                char *new_arg = malloc(strlen(prefix) + strlen(var_value) + 1);
                strcpy(new_arg, prefix);
                strcat(new_arg, var_value);
                
                my_argv[i] = new_arg; 
                is_malloced[i] = 1;   
            }
        }

        char *exec_argv[100];
        int exec_argc = 0;
        char *in_file = NULL;
        char *out_file = NULL;
        char *err_file = NULL;
        int redir_error = 0;

        for (int i = 0; i < my_argc; i++) {
            if (strcmp(my_argv[i], "<") == 0 || strcmp(my_argv[i], ">") == 0 || strcmp(my_argv[i], "2>") == 0) {
                if (i + 1 >= my_argc) {
                    printf("Error: Missing filename for redirection\n");
                    redir_error = 1;
                    break;
                }
                if (strcmp(my_argv[i], "<") == 0) in_file = my_argv[i+1];
                else if (strcmp(my_argv[i], ">") == 0) out_file = my_argv[i+1];
                else if (strcmp(my_argv[i], "2>") == 0) err_file = my_argv[i+1];
                i++; 
            } else {
                exec_argv[exec_argc++] = my_argv[i];
            }
        }
        exec_argv[exec_argc] = NULL;

        if (redir_error) {
            for (int i = 0; i < my_argc; i++) { if (is_malloced[i] == 1) free(my_argv[i]); }
            free(my_argv);
            continue;
        }

        int saved_in = dup(STDIN_FILENO);
        int saved_out = dup(STDOUT_FILENO);
        int saved_err = dup(STDERR_FILENO);
        int fd_err = 0;

        if (in_file) {
            int fd = open(in_file, O_RDONLY);
            if (fd < 0) { perror("open"); fd_err = 1; }
            else { dup2(fd, STDIN_FILENO); close(fd); }
        }
        if (out_file && !fd_err) {
            int fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("open"); fd_err = 1; }
            else { dup2(fd, STDOUT_FILENO); close(fd); }
        }
        if (err_file && !fd_err) {
            int fd = open(err_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("open"); fd_err = 1; }
            else { dup2(fd, STDERR_FILENO); close(fd); }
        }

        if (fd_err) {
            dup2(saved_in, STDIN_FILENO); dup2(saved_out, STDOUT_FILENO); dup2(saved_err, STDERR_FILENO);
            close(saved_in); close(saved_out); close(saved_err);
            for (int i = 0; i < my_argc; i++) { if (is_malloced[i] == 1) free(my_argv[i]); }
            free(my_argv);
            continue;
        }

        if (exec_argc == 0) {
            dup2(saved_in, STDIN_FILENO); dup2(saved_out, STDOUT_FILENO); dup2(saved_err, STDERR_FILENO);
            close(saved_in); close(saved_out); close(saved_err);
            for (int i = 0; i < my_argc; i++) { if (is_malloced[i] == 1) free(my_argv[i]); }
            free(my_argv);
            continue;
        }

        if (strchr(exec_argv[0], '=') != NULL) {
            if (exec_argc != 1 || exec_argv[0][0] == '=' || exec_argv[0][strlen(exec_argv[0]) - 1] == '=') {
                printf("Invalid command\n");
            } else {
                char *eq_pos = strchr(exec_argv[0], '=');
                *eq_pos = '\0'; 
                
                char *var_name = exec_argv[0];
                char *var_value = eq_pos + 1;
                
                set_local_var(var_name, var_value);
            }
        }
        
        else if (strcmp(exec_argv[0], "echo") == 0) {
            for (int i = 1; i < exec_argc; i++) {
                if (i == (exec_argc - 1)) {
                    printf("%s", exec_argv[i]);   
                } else {
                    printf("%s ", exec_argv[i]);
                }
            }
            printf("\n");
        }
        
        else if (strcmp(exec_argv[0], "export") == 0) {
            if (exec_argc < 2) {
                printf("export: missing argument\n");
            } else {
                char *val = get_any_var(exec_argv[1]);
                if (strcmp(val, "") != 0) {
                    setenv(exec_argv[1], val, 1);
                }
            }
        }
        
        else if (strcmp(exec_argv[0], "exit") == 0) {
            printf("Good Bye\n");
            free_local_vars(); 
            for (int i = 0; i < my_argc; i++) { if (is_malloced[i] == 1) free(my_argv[i]); }
            free(my_argv);     
            break;
        }
        
        else if (strcmp(exec_argv[0], "pwd") == 0) {
            char my_array[4096];    
            char *x = getcwd(my_array, 4096);
            if (x == NULL) {
                printf("Error pwd failed\n");
            } else {
                printf("%s\n", my_array);
            }
        }
        
        else if (strcmp(exec_argv[0], "cd") == 0) {
            if (exec_argc < 2 || strcmp(exec_argv[1], "~") == 0) {
                char *home_dir = getenv("HOME");
                if (home_dir != NULL && chdir(home_dir) != 0) {
                    printf("error\n");
                }
            } else if (chdir(exec_argv[1]) != 0) {
                printf("error\n");
            }
        }
        
        else {
            int pid = fork();
            if (pid < 0) {
                printf("Error: Fork failed\n");
            } else if (pid == 0) {
                execvp(exec_argv[0], exec_argv);
                printf("Invalid command\n");
                return 1; 
            } else {
                wait(NULL);
            }
        }
        
        dup2(saved_in, STDIN_FILENO);
        dup2(saved_out, STDOUT_FILENO);
        dup2(saved_err, STDERR_FILENO);
        close(saved_in);
        close(saved_out);
        close(saved_err);
        
        for (int i = 0; i < my_argc; i++) {
            if (is_malloced[i] == 1) {
                free(my_argv[i]);
            }
        }
        free(my_argv); 
    }
    
    return 0;
}
