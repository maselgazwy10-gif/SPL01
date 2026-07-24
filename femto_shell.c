#include <stdio.h>
#include <string.h>

int main() {
    char input[65536];

    while (1) {
        printf("FemtoShell waiting for your command ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break; 
        }

        input[strcspn(input, "\n")] = '\0';

        char *command = strtok(input, " ");

        if (command == NULL) {
            continue;
        }

        if (strcmp(command, "echo") == 0) {
            char *argument = strtok(NULL, " ");
            while (argument != NULL) {
                printf("%s", argument);
                
                argument = strtok(NULL, " ");
                if (argument != NULL) {
                    printf(" ");
                }
            }
            printf("\n");
        } 
        else if (strcmp(command, "exit") == 0) {
            printf("Good Bye :)\n");
            break;
        } 
        else {
            printf("Invalid command\n");
        }
    }

    return 0;
}
