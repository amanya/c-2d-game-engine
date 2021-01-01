//
// Created by albert on 1/1/21.
//

#include <stdlib.h>
#include <string.h>

#include "list.h"

void list_init(List *list, void (*destroy)(void *data)) {
    list->size = 0;
    list->destroy = destroy;
    list->head = NULL;
    list->tail = NULL;
}

void list_destroy(List *list) {
    void *data;

    while (list_size(list) > 0) {
        if (list_rem_next(list, NULL, (void **)&data) == 0 && list->destroy != NULL) {
            list->destroy(data);
        }
    }

    memset(list, 0, sizeof(List));
}

int list_is_member(List *list, const void *data) {
    ListItem *pointer = list_head(list);

    while (pointer) {
        if (list_data(pointer) == data) {
            return 1;
        }
        pointer = list_next(pointer);
    }
    return 0;
}

int list_ins_next(List *list, ListItem *item, const void *data) {
    ListItem *new_item;

    if ((new_item = (ListItem *)malloc(sizeof(ListItem))) == NULL)
        return -1;

    new_item->data = (void *)data;

    if (item == NULL) {
        if (list_size(list) == 0)
            list->tail = new_item;

        new_item->next = list->head;
        list->head = new_item;
    } else {
        if (item->next == NULL)
            list->tail = new_item;

        new_item->next = item->next;
        item->next = new_item;
    }

    list->size++;

    return 0;
}

int list_rem_next(List *list, ListItem *item, void **data) {
    ListItem *old_item;

    if (list_size(list) == 0)
        return -1;

    if (item == NULL) {
        *data = list->head->data;
        old_item = list->head;
        list->head = list->head->next;

        if (list_size(list) == 1)
            list->tail = NULL;
    } else {
        if (item->next == NULL)
            return -1;

        *data = item->next->data;
        old_item = item->next;
        item->next = item->next->next;

        if (item->next == NULL)
            list->tail = item;
    }

    free(old_item);

    list->size--;

    return 0;
}
