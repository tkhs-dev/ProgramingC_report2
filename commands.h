#ifndef PROGRAMINGC_REPORT2_COMMANDS_H
#define PROGRAMINGC_REPORT2_COMMANDS_H

#endif //PROGRAMINGC_REPORT2_COMMANDS_H

typedef struct {
    char *name;
    bool (*resolver_function)(char *);
    int (*executor_function)(char *[]);
} command;

command *select_command(char *command);

