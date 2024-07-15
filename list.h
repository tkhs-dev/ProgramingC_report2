#define foreach(item, list) \
    for(LIST *item = list; item != NULL; item = item->next)

typedef struct list{
    void*  content;
    struct list *prev;
    struct list *next;
} LIST;

LIST *new_item(void* content, LIST *post_item, LIST *pre_item);
LIST *add(LIST *root, void* content);
LIST *delete(LIST *item);
void clear_list(LIST *item);
