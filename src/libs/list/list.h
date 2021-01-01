//
// Created by albert on 1/1/21.
//

#ifndef SDLSCROLLER_LIST_H
#define SDLSCROLLER_LIST_H

#include <stdlib.h>

typedef struct LIST_ITEM {
    void *data;
    struct LIST_ITEM *next;
} ListItem;

typedef struct LIST {
    int size;

    int (*match)(const void *key1, const void *key2);
    void (*destroy)(void *data);

    ListItem *head;
    ListItem *tail;
} List;

void list_init(List *list, void (*destroy)(void *data));
void list_destroy(List *list);
int list_is_member(List *list, const void *data);
int list_ins_next(List *list, ListItem *item, const void *data);
int list_rem_next(List *list, ListItem *item, void **data);
#define list_size(list) ((list)->size)
#define list_head(list) ((list)->head)
#define list_tail(list) ((list)->tail)
#define list_is_head(list, item) ((item) == (list)->head ? 1 : 0)
#define list_is_tail(item) ((item)->next == NULL ? 1 : 0)
#define list_data(item) ((item)->data)
#define list_next(item) ((item)->next)

#endif //SDLSCROLLER_LIST_H
