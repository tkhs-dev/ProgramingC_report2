#ifndef PROGRAMINGC_REPORT2_LIST_H
#define PROGRAMINGC_REPORT2_LIST_H

#endif //PROGRAMINGC_REPORT2_LIST_H

typedef struct str_list{
    char            *content;
    struct str_list *prev;
    struct str_list *next;
} STR_LIST;

STR_LIST *new_item(char *content, STR_LIST *post_item, STR_LIST *pre_item);
STR_LIST *insert(STR_LIST *root, char *content);
STR_LIST *delete(STR_LIST *item);
void clear_list(STR_LIST *item);
