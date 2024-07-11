#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>
#include "commands.h"

/*
 * コマンドの実装
 */
int cd_executor(char *args[]) {
    printf("cd called!!\n");
    return 0;
}

int pushd_executor(char *args[]) {
    printf("pushd called!!\n");
    return 0;
}

int dirs_executor(char *args[]) {
    printf("dirs called!!\n");
    return 0;
}

int popd_executor(char *args[]) {
    printf("popd called!!\n");
    return 0;
}

int history_executor(char *args[]) {
    printf("history called!!\n");
    return 0;
}

int redo_executor(char *args[]) {
    printf("redo called!!\n");
    return 0;
}

bool redo_resolver(char *command) {
    return strcmp(command, "!!") == 0;
}

int prompt_executor(char *args[]) {
    printf("prompt called!!\n");
    return 0;
}

int alias_executor(char *args[]) {
    printf("alias called!!\n");
    return 0;
}


/*
 * コマンドの選択
 */

command commands[] = {
        {"cd",NULL, cd_executor},
        {"pushd",NULL, pushd_executor},
        {"dirs",NULL, dirs_executor},
        {"popd",NULL, popd_executor},
        {"history",NULL, history_executor},
        {"!!",redo_resolver, redo_executor},
        {"prompt",NULL, prompt_executor},
        {"alias",NULL, alias_executor},
};

command* select_command(char *command) {
    for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if(commands[i].resolver_function == NULL){
            if (strcmp(commands[i].name, command) == 0) {
                return &commands[i];
            }else{
                continue;
            }
        }else if (commands[i].resolver_function(command)) {
            return &commands[i];
        }
    }
    return NULL;
}
