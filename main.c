#include <stdio.h>
#include <string.h>

#define SHELL_NAME "MyShell" // Macro definition";

int main() {
    printf("Welcome to %s!!\n", SHELL_NAME);
    printf("Type 'exit' to exit\n");
    printf("$");
    char command[100];
    while (1) {
        fgets(command, 100, stdin);
        if (strcmp(command, "exit") == 0) {
            break;
        }
        printf("Command not found\n");
        printf("$");
    }
    return 0;
}

void parse_command(char *command) {
    // Parse the command here
}
