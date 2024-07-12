#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

STR_LIST *new_item(char *content, STR_LIST *post_item, STR_LIST *pre_item)
{
    STR_LIST *new;

    new = (STR_LIST *)malloc(sizeof(STR_LIST));

    if (!new) {
        perror("new_item");
        exit(1);
    }

    if (content) {
        new->content = strdup(content);
        if (!new->content) {
            perror("new_item");
            exit(1);
        }
    } else {
        new->content = content;
    }
    new->next = post_item;
    if (post_item)
        post_item->prev = new;
    new->prev = pre_item;
    if (pre_item)
        pre_item->next = new;

    return new;
}

STR_LIST *insert(STR_LIST *root, char *content){

    STR_LIST *p;

    /* root が NULL の場合の新規リスト要素の挿入処理 */
    if(root == NULL)
        return(new_item(content, NULL, NULL));

    return(new_item(content, root, NULL));
}

STR_LIST *delete(STR_LIST *item){
    STR_LIST *next_item = item->next;
    STR_LIST *prev_item = item->prev;

    if(prev_item)
        prev_item->next = next_item;
    if(next_item)
        next_item->prev = prev_item;

    free(item->content);
    free(item);

    return next_item;
}

void clear_list(STR_LIST *item){
    if(item == NULL)
        return;
    clear_list(item->next);
    free(item->content);
    free(item);
}
