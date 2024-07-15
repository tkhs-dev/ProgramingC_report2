#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

LIST *new_item(void* content, LIST *post_item, LIST *pre_item)
{
    LIST *new;

    new = (LIST *)malloc(sizeof(LIST));

    if (!new) {
        perror("new_item");
        exit(1);
    }

    if (content) {
        new->content = content;
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

LIST *insert(LIST *root, void* content){
    if(root == NULL)
        return(new_item(content, NULL, NULL));

    return(new_item(content, root, NULL));
}

LIST *delete(LIST *item){
    LIST *next_item = item->next;
    LIST *prev_item = item->prev;

    if(prev_item)
        prev_item->next = next_item;
    if(next_item)
        next_item->prev = prev_item;

    free(item->content);
    free(item);

    return next_item;
}

void clear_list(LIST *item){
    if(item == NULL)
        return;
    clear_list(item->next);
    free(item->content);
    free(item);
}
