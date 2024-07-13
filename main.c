/*----------------------------------------------------------------------------
 *  簡易版シェル
 *--------------------------------------------------------------------------*/

/*
 *  インクルードファイル
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include "list.h"

/*
 *  定数の定義
 */

#define BUFLEN    1024     /* コマンド用のバッファの大きさ */
#define MAXARGNUM  256     /* 最大の引数の数 */
#define HISTORY_SIZE 32

/*
 * 構造体の定義
 */

typedef struct {
    char *name;
    bool (*is_match_func)(char *);
    int (*executor)(char *[]);
} command;

typedef struct {
    int index;
    char *command;
} history;

typedef struct {
    char *name;
    char *value;
} alias;


/*
 *  ローカルプロトタイプ宣言
 */

void initialize_history(int);
void add_history(char *);
void dispose_history();
void dispose_alias();
int parse(char [], char *[]);
void execute_command(char *[], int);
command *select_command(char *command);
bool redo_match(char *command);

/*
 * ヒストリー系の変数
 */
struct list *history_list;

/*
 * ディレクトリスタック系の変数
 */
struct list *dir_stack;

/*
 * エイリアス系の変数
 */
struct list *alias_list;

/*----------------------------------------------------------------------------
 *
 *  関数名   : main
 *
 *  作業内容 : シェルのプロンプトを実現する
 *
 *  引数     :
 *
 *  返り値   :
 *
 *  注意     :
 *
 *--------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    char command_buffer[BUFLEN]; /* コマンド用のバッファ */
    char *args[MAXARGNUM];       /* 引数へのポインタの配列 */
    int command_status;          /* コマンドの状態を表す
                                    command_status = 0 : フォアグラウンドで実行
                                    command_status = 1 : バックグラウンドで実行
                                    command_status = 2 : シェルの終了
                                    command_status = 3 : 何もしない */

    initialize_history(HISTORY_SIZE);

    /*
     *  無限にループする
     */

    for(;;) {

        /*
         *  プロンプトを表示する
         */
        char* prompt = getenv("PSC1");
        if (prompt == NULL) {
            prompt = "Command : ";
        }
        printf("%s",prompt);

        /*
         *  標準入力から１行を command_buffer へ読み込む
         *  入力が何もなければ改行を出力してプロンプト表示へ戻る
         */

        if(fgets(command_buffer, BUFLEN, stdin) == NULL) {
            printf("\n");
            continue;
        }

        /*
         *  入力されたバッファ内のコマンドを解析する
         *
         *  返り値はコマンドの状態
         */
        char *command = strdup(command_buffer);
        command_status = parse(command, args);

        /*
         *  終了コマンドならばプログラムを終了
         *  引数が何もなければプロンプト表示へ戻る
         */

        if(command_status == 2) {
            printf("done.\n");
            break;
        } else if(command_status == 3) {
            continue;
        }

        /*
         *  コマンドを実行する
         */

        execute_command(args, command_status);
        printf("\n");

        /*
        *  ヒストリーに追加
        */
        if(redo_match(command_buffer) == false){
            add_history(command_buffer);
        }
    }
    dispose_history();
    clear_list(dir_stack);
    dispose_alias(alias_list);
    exit(EXIT_SUCCESS);
}

void initialize_history(int history_size) {
    history_list = NULL;
    history_list = insert(history_list, NULL);
    struct list *tail = history_list;
    for(int i = 0; i < history_size-2; i++) {
        history_list = insert(history_list, NULL);
    }
    history_list = new_item(NULL, history_list, tail);
}

void add_history(char *command) {
    history *new_history = (history *)malloc(sizeof(history));
    new_history->command = strdup(command);
    if(history_list->content != NULL) {
        free(((history *)history_list->content)->command);
        free(history_list->content);
    }
    if(history_list->prev->content != NULL) {
        new_history->index = ((history *)history_list->prev->content)->index + 1;
    }else{
        new_history->index = 0;
    }
    history_list->content = new_history;
    history_list = history_list->next;
}

history* get_history_absolutely(int index){
    struct list *current = history_list;
    while(1) {
        if(current->content != NULL && ((history *)current->content)->index == index){
            return current->content;
        }
        current = current->next;
        if(current == history_list){
            break;
        }
    }
    return NULL;
}

void dispose_history() {
    struct list *current = history_list;
    while(1) {
        if(current == history_list){
            break;
        }
        if(current->content != NULL){
            free(((history *)current->content)->command);
        }
        current = current->next;
    }
    history_list->prev->next = NULL;
    clear_list(history_list);
}

history* get_history_relatively(int n){
    struct list *current = history_list;
    for(int i = 0; i < n; i++) {
        current = current->prev;
    }
    return current->content;
}

char* get_alias(char *name) {
    struct list *current = alias_list;
    while(1) {
        if(current == NULL){
            break;
        }
        current = current->next;
    }
    current = alias_list;
    while(1) {
        if(current == NULL){
            break;
        }
        if(current->content != NULL && ((alias *)current->content)->name != NULL && strcmp(((alias *)current->content)->name, name) == 0){
            return ((alias *)current->content)->value;
        }
        current = current->next;
    }
    return NULL;
}

void dispose_alias() {
    struct list *current = alias_list;
    while(1) {
        if(current == NULL){
            break;
        }
        if(current->content != NULL){
            free(((alias *)current->content)->name);
            free(((alias *)current->content)->value);
        }
        current = current->next;
    }
    clear_list(alias_list);
}


/*----------------------------------------------------------------------------
 *
 *  関数名   : parse
 *
 *  作業内容 : バッファ内のコマンドと引数を解析する
 *
 *  引数     :
 *
 *  返り値   : コマンドの状態を表す :
 *                0 : フォアグラウンドで実行
 *                1 : バックグラウンドで実行
 *                2 : シェルの終了
 *                3 : 何もしない
 *
 *  注意     :
 *
 *--------------------------------------------------------------------------*/

int parse(char buffer[],        /* バッファ */
          char *args[])         /* 引数へのポインタ配列 */
{
    int arg_index;   /* 引数用のインデックス */
    int status;   /* コマンドの状態を表す */
    bool in_quotes = false;
    /*
     *  変数の初期化
     */

    arg_index = 0;
    status = 0;

    /*
     *  バッファ内の最後にある改行をヌル文字へ変更
     */

    *(buffer + (strlen(buffer) - 1)) = '\0';

    /*
     *  バッファが終了を表すコマンド（"exit"）ならば
     *  コマンドの状態を表す返り値を 2 に設定してリターンする
     */

    if(strcmp(buffer, "exit") == 0) {

        status = 2;
        return status;
    }

    /*
     *  バッファ内の文字がなくなるまで繰り返す
     *  （ヌル文字が出てくるまで繰り返す）
     */

    while(*buffer != '\0') {

        /*
         *  空白類（空白とタブ）をヌル文字に置き換える
         *  これによってバッファ内の各引数が分割される
         */

        while(*buffer == ' ' || *buffer == '\t') {
            *(buffer++) = '\0';
        }

        /*
         * 空白の後が終端文字であればループを抜ける
         */

        if(*buffer == '\0') {
            break;
        }

        /*
         *  引数の先頭が " で始まっている場合
         *  " が終わるまでの間は空白類を無視する
         */
        if (*buffer == '\"') {
            in_quotes = !in_quotes;
            buffer++;
        }

        /*
         *  空白部分は読み飛ばされたはず
         *  buffer は現在は arg_index + 1 個めの引数の先頭を指している
         *
         *  引数の先頭へのポインタを引数へのポインタ配列に格納する
         */

        args[arg_index] = buffer;
        ++arg_index;

        /*
         *  引数部分を読み飛ばす
         *  （ヌル文字でも空白類でもない場合に読み進める）
         */

        while((*buffer != '\0') && (in_quotes || (*buffer != ' ' && *buffer != '\t'))) {
            if (*buffer == '\"') {
                in_quotes = !in_quotes;
                *(buffer++) = '\0';
                break;
            }
            ++buffer;
        }
    }

    /*
     *  最後の引数の次にはヌルへのポインタを格納する
     */

    args[arg_index] = NULL;

    /*
     *  最後の引数をチェックして "&" ならば
     *
     *  "&" を引数から削る
     *  コマンドの状態を表す status に 1 を設定する
     *
     *  そうでなければ status に 0 を設定する
     */

    if(arg_index > 0 && strcmp(args[arg_index - 1], "&") == 0) {

        --arg_index;
        args[arg_index] = '\0';
        status = 1;

    } else {

        status = 0;

    }

    /*
     *  引数が何もなかった場合
     */

    if(arg_index == 0) {
        status = 3;
    }

    /*
     *  コマンドの状態を返す
     */

    return status;
}

/*----------------------------------------------------------------------------
 *
 *  関数名   : execute_command
 *
 *  作業内容 : 引数として与えられたコマンドを実行する
 *             コマンドの状態がフォアグラウンドならば、コマンドを
 *             実行している子プロセスの終了を待つ
 *             バックグラウンドならば子プロセスの終了を待たずに
 *             main 関数に返る（プロンプト表示に戻る）
 *
 *  引数     :
 *
 *  返り値   :
 *
 *  注意     :
 *
 *--------------------------------------------------------------------------*/

void execute_command(char *args[],    /* 引数の配列 */
                     int command_status)     /* コマンドの状態 */
{
    command *command = select_command(args[0]);
    if (command != NULL) {
        command->executor(args);
        return;
    }else{
        int pid;      /* プロセスＩＤ */
        int status;   /* 子プロセスの終了ステータス */

        /*
         *  子プロセスを生成する
         *
         *  生成できたかを確認し、失敗ならばプログラムを終了する
         */
        pid = fork();
        if(pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        /*
         *  子プロセスの場合には引数として与えられたものを実行する
         *
         *  引数の配列は以下を仮定している
         *  ・第１引数には実行されるプログラムを示す文字列が格納されている
         *  ・引数の配列はヌルポインタで終了している
         */
        if(pid == 0) {
            status = execvp(args[0], args);
            if(status < 0) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }else{
            /*
            *  コマンドの状態がバックグラウンドなら関数を抜ける
            */

            if(command_status == 1) {
                return;
            }

            /*
             *  ここにくるのはコマンドの状態がフォアグラウンドの場合
             *
             *  親プロセスの場合に子プロセスの終了を待つ
             */
            wait(&status);
        }


        return;
    }
}

/*
 * コマンドの実装
 */
int cd_executor(char *args[]) {
    if(args[1] == NULL){
        chdir(getenv("HOME"));
        return 0;
    }else{
        if(chdir(args[1]) < 0){
            perror("cd");
            return 1;
        }
        return 0;
    }
}

int pushd_executor(char *args[]) {
    dir_stack = insert(dir_stack, getcwd(NULL, 0));
    return 0;
}

int dirs_executor(char *args[]) {
    struct list *current = dir_stack;
    while(1) {
        if(current == NULL){
            break;
        }
        printf("%s\n", (char *)current->content);
        current = current->next;
    }
    return 0;
}

int popd_executor(char *args[]) {
    if(dir_stack == NULL){
        printf("Directory stack is empty\n");
        return 1;
    }
    chdir((char *)dir_stack->content);
    dir_stack = delete(dir_stack);

    return 0;
}

int history_executor(char *args[]) {
    struct list *current = history_list;
    struct list *head = current;
    while(1) {
        if(current->content != NULL){
            printf("[%d] %s", ((history *)current->content)->index, ((history *)current->content)->command);
        }
        current = current->next;
        if(current == head) {
            break;
        }
    }
    return 0;
}

int redo_executor(char *args[]) {
    history* his = NULL;
    if(args[0][1] == '!'){
        his = get_history_relatively(1);
    }else{
        char *e;
        int index = strtol(args[0]+1, &e, 10);
        if(*e == '\0'){
            if(index < -HISTORY_SIZE || index >= HISTORY_SIZE){
                printf("Invalid history index\n");
                return 1;
            }
            his = index >= 0 ? get_history_absolutely(index) : get_history_relatively(-index);
        }else{
            struct list *current = history_list;
            while(1) {
                if (current->content != NULL &&
                    strncmp(((history *) current->content)->command, args[0] + 1, strlen(args[0] + 1)) == 0) {
                    his = current->content;
                    break;
                }
                current = current->next;
                if (current == history_list) {
                    break;
                }
            }
        }
    }

    if(his == NULL){
        printf("No such history\n");
        return 1;
    }
    char* command = strdup(his->command);

    char *redo_args[MAXARGNUM];
    int status = parse(command, redo_args);
    execute_command(redo_args, status);
    free(command);
    return 0;
}

bool redo_match(char *command) {
    return command[0] == '!';
}

int prompt_executor(char *args[]) {
    setenv("PSC1", args[1], 1);
    return 0;
}

int alias_executor(char *args[]) {
    if(args[1] == NULL || args[2] == NULL){
        printf("alias: Too few arguments\n");
        return 1;
    }
    alias* new_alias = (alias *)malloc(sizeof(alias));
    new_alias->name = strdup(args[1]);
    new_alias->value = strdup(args[2]);
    alias_list = insert(alias_list, new_alias);
    return 0;
}

int unalias_executor(char *args[]) {
    struct list *current = alias_list;
    while(1) {
        if(current == NULL){
            break;
        }
        if(current->content != NULL && strcmp(((alias *)current->content)->name, args[1]) == 0){
            free(((alias *)current->content)->name);
            free(((alias *)current->content)->value);
            alias_list = delete(current);
            break;
        }
        current = current->next;
    }
    return 0;
}


/*
 * コマンドの設定
 */

command commands[] = {
        {"cd",NULL,        cd_executor},
        {"pushd",NULL,     pushd_executor},
        {"dirs",NULL,      dirs_executor},
        {"popd",NULL,      popd_executor},
        {"history",NULL,   history_executor},
        {"!!", redo_match, redo_executor},
        {"prompt",NULL,    prompt_executor},
        {"alias",NULL,     alias_executor},
        {"unalias",NULL,   unalias_executor},
};

command* select_command(char *command) {
    char *alias_command = get_alias(command);
    if(alias_command != NULL){
        strcpy(command, alias_command);
    }
    for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if(commands[i].is_match_func == NULL){
            if (strcmp(commands[i].name, command) == 0) {
                return &commands[i];
            }else{
                continue;
            }
        }else if (commands[i].is_match_func(command)) {
            return &commands[i];
        }
    }
    return NULL;
}

/*-- END OF FILE -----------------------------------------------------------*/
