//
// Created by root on 4/29/23.
//

#include "snake_list.h"

struct linked_list_t *listCreate(void) {
    struct linked_list_t *list = calloc(1, sizeof(struct linked_list_t));
    list->size = INITIAL_SIZE;

    list->head = malloc(sizeof(struct node_t));
    list->head->next = malloc(sizeof(struct node_t));
    list->head->next->next = malloc(sizeof(struct node_t));

    list->head->component.x = INITIAL_HEAD_POS_X;
    list->head->component.y = INITIAL_HEAD_POS_Y;
    list->head->next->component.x = INITIAL_HEAD_POS_X - 1;
    list->head->next->component.y = INITIAL_HEAD_POS_Y;
    list->head->next->next->component.x = INITIAL_HEAD_POS_X - 2;
    list->head->next->next->component.y = INITIAL_HEAD_POS_Y;

    list->tail = list->head->next->next;

    return list;
}

int list_push(struct linked_list_t *list, int x, int y) {
    list->tail->next = malloc(sizeof(struct node_t));
    list->tail = list->tail->next;
    list->tail->component.x = x;
    list->tail->component.y = y;

    list->size++;

    return 0;
}

struct node_t *list_get_node_by_index(struct linked_list_t *list, int index) {
    struct node_t *node = list->head;

    for (int i = 0; i < index; i++)
        node = node->next;

    return node;
}
